#include "test.pb.h"
#include <iostream>
#include <string>
/*
int main()
{
    fixbug::LoginRequest req;
    req.set_name("ldy");
    req.set_pwd("123");

    //对象数据序列化-》string*
    std::string send_str;
    if(req.SerializeToString(&send_str))
    {
        std::cout<<send_str<<std::endl;
    }

    //从send_str反序列化得到一个LoginRequest对象
    fixbug::LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout<<reqB.name()<<" "<<reqB.pwd()<<std::endl;
    }

    return 0;
}
*/
int main()
{
    fixbug::LoginResponse rsp;
    fixbug::ResultCode *rc= rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("errno happen!");

    fixbug::GetListResponse rsp2;
    fixbug::ResultCode *rc2=rsp2.mutable_result();
    rc2->set_errcode(0);
    fixbug::user* user1=rsp2.add_frined_list();
    user1->set_name("ely");
    user1->set_age(18);
    user1->set_sex(fixbug::user::MAN);
    std::cout<<rsp2.frined_list_size()<<std::endl;
    return 0;
}