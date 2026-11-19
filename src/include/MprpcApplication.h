#pragma once

#include "MprpcConfig.h"
#include "MprpcChannel.h"
#include "MprpcController.h"

// mprpc框架基础类，负责初始化
class MprpcApplication
{
private:
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;
    MprpcApplication &operator=(const MprpcApplication &) = delete;
    MprpcApplication &operator=(MprpcApplication &&) = delete;

    static MprpcConfig m_config;
public:
    ~MprpcApplication() = default;
    static void Init(int argc, char **argv);
    static MprpcApplication &GetInstance();
    static MprpcConfig& GetConfig();
};
