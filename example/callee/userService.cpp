#include <iostream>
#include <string>
#include "user.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"

// UserService是一个本地服务，提供了两个进程内的本地方法Login和GetFriendList
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务提供者
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id << ",name:" << name << ",pwd:" << pwd << std::endl;
        return true;
    }

    /*
    **重写基类虚函数:
      1.caller -> Login(LoginRequest) ->muduo ->callee
      2.callee->Login(LoginRequest)->下面这个函数
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 底层给业务上报了请求参数LoginRequest，应用层获取对应的数据
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name, pwd); // 本地处理

        // 写入响应
        response->set_success(login_result);
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");

        // 执行回调：执行响应对象数据的序列化和网络发送
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();

        bool ret=Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化
    MprpcApplication::Init(argc, argv);

    // 把UserService对象发布到rpc节点上，provider是一个rpc网络服务对象
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点 进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
    return 0;
}
