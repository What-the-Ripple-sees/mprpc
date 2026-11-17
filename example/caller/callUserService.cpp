#include <iostream>
#include "MprpcApplication.h"
#include "user.pb.h"
#include "MprpcChannel.h"
int main(int argc,char** argv)
{
    //整个程序启动以后，想使用mprpc框架调用 rpc服务，一定先需要调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远端发布的rpc服务方法
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;

    //发起rpc方法调用
    stub.Login(nullptr,&request,&response,nullptr);//会调用google::protobuf::RpcChannel::CallMethod(RpcChannel是一个纯虚类)

    //读取响应
    if(response.result().errcode()==0)
    {
        std::cout<<"rpc login response:"<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
    }
    return 0;
}