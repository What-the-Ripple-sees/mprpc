#pragma once
#include <google/protobuf/service.h>
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

//框架提供的用于发布rpc服务的网络对象类
class RpcProvider
{
private:
    
    muduo::net::EventLoop m_eventLoop;

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务的所有方法
    };

    //存储所有注册成功的服务对象和其服务的所有方法 
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;

    //新连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //消息读写回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,
                            muduo::net::Buffer*,
                            muduo::Timestamp);

    //Closure回调操作，用于序列化rpc响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response);
public:

    //提供给外部使用，发布rpc服务的函数接口
    void NotifyService(google::protobuf::Service* service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
};

