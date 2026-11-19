#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "MprpcChannel.h"
#include "rpcheader.pb.h"
#include "MprpcApplication.h"
#include "MprpcController.h"

/*
    RpcProvider和RpcConsumer协商二者通信的protobuf数据类型：包括service_name method_name args这些字段，args时method对应方法的参数

    定义格式为：header_size+header_str+args_str（要考虑粘包问题）
    header_size 指定header_str长度，header_size字段的长度固定为4字节
    header_str包括service_name,method_name和后续args_str的长度

    见src/rpcheader.proto

    故一次rpc调用请求的字节流为：
    header_size(固定4bytes)+header_str(包括service_name+method_name+args_size)+args_str
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数序列化字符串长度,request中只有要调用的rpc方法的参数
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        //std::cout << "serialize request error!" << std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        //std::cout << "serialize rpc header error!" << std::endl;
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送rpc请求的全部字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, (char *)&header_size, 4);
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "==========================================" << std::endl;

    //使用tcp编程，完成数据发送
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd)
    {
        //std::cout<<"create socket error! errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"create socket error!errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    //读取配置文件中rpcserver信息并填入
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = std::stoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport"));
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());

    //连接rpc服务节点
    if(-1==connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        //std::cout<<"connect error! errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"connect error! errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    //发送rpc请求
    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        //std::cout<<"send error! errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"send error! errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    
    //接收rpc请求响应
    char recv_buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,recv_buf,1024,0)))
    {
        //std::cout<<"recv error! errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"recv error! errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    //std::string recv_buf_str(0,recv_buf,4);//这里用string会出现碰到/0导致提前写入结束？
    //if(!response->ParseFromString(recv_buf,recv_size))
    if(!response->ParseFromArray(recv_buf,recv_size))
    {
        //std::cout<<"parse error! response_str:"<<recv_buf<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"parse error! response_str:%s",recv_buf);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    close(clientfd);
}