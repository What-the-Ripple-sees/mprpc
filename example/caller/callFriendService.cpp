#include <iostream>
#include "MprpcApplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架调用 rpc服务，一定先需要调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远端发布的rpc服务方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    fixbug::GetFriendListRequest request;
    request.set_userid(1000);

    fixbug::GetFriendListResponse response;

    MprpcController controller;

    // 以同步方式发起rpc方法调用
    stub.GetFriendList(&controller, &request, &response, nullptr); // 会调用google::protobuf::RpcChannel::CallMethod(RpcChannel是一个纯虚类)

    if (controller.Failed())
    {
        // rpc调用未成功
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        // 读取响应
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc GetFriendList response success:" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                std::cout << "index:" << (i + 1) << ",name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendList response error:" << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}