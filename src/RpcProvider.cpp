#include "RpcProvider.h"
#include "MprpcApplication.h"
#include "rpcheader.pb.h"

// 提供给外部使用，发布rpc服务的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象的方法数量
    int methodCount = pserviceDesc->method_count();

    for (int i = 0; i < methodCount; ++i)
    {
        // 获取服务对象的所有服务方法
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = std::stoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);

    // 创建muduo::net::TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库线程数量
    server.setThreadNum(3);

    std::cout << "RpcProvider::run() start service at ip:" << ip << ",port:" << port << std::endl;
    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 连接已断开时
        conn->shutdown();
    }
}

// Closure回调操作，用于序列化rpc响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))//进行序列化
    {
        //序列化成功后，通过rpc方法把执行的结果发送回rpc调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"serialize response_str error!"<<std::endl;
    }
    conn->shutdown();//模拟http短链接服务，由rpcprovider主动断开连接
}

/*
    RpcProvider和RpcConsumer协商二者通信的protobuf数据类型：包括service_name method_name args这些字段，args时method对应方法的参数

    定义格式为：header_size+header_str+args_str（要考虑粘包问题）
    header_size 指定header_str长度，header_size字段的长度固定为4字节
    header_str包括service_name,method_name和后续args_str的长度

    见src/rpcheader.proto

    故一次rpc调用请求的字节流为：
    header_size(固定4bytes)+header_str(包括service_name+method_name+args_size)+args_str
*/
// 当远端有rpc请求发送时，本端接收到请求并进入该函数
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    // 网络上接收的远程rpc请求的字节流获取
    std::string recv_buf = buffer->retrieveAllAsString();

    // 读取前四个字节
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size读取header
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << "parse error!" << std::endl;
        return;
    }

    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "==========================================" << std::endl;

    // 获取service,method
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << method_name << " is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      // 获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 构造rpc方法调用的请求和响应参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << " request parse error ! content:" << args_str << std::endl;
        return;
    }

    // 创建service->CallMethod的Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback
                                        <RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>
                                        (this, &RpcProvider::SendRpcResponse, conn, response);
    // 根据远端rpc请求，调用本端发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}