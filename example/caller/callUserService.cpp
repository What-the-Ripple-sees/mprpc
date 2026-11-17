#include <iostream>
#include "MprpcApplication.h"
#include "user.pb.h"
int main(int argc,char** argv)
{
    //整个程序启动以后，想使用mprpc框架调用 rpc服务，一定先需要调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远端发布的rpc服务方法
    //fixbug::UserServiceRpc_Stub stub(RpcChannel*);
    return 0;
}