#include "header.h"

//登录/注册动作函数
//第一版第二版
//将从客户端发来的用户名和密码进行验证
//如果是登录行为，密码输入不正确，要求重试
//如果是注册行为，用户名已经存在，则失败
int loginRegisterSystem(train_t *t, int net_fd)
{
    while(1)
    {
        bzero(t, sizeof(train_t));
        //先接收一次的登录信息或者注册信息;
        ssize_t rret = recv(net_fd, t, sizeof(train_t), MSG_WAITALL);
        ERROR_CHECK(rret, -1, "对端关闭");

        //接收密码
        char password[1024] = {0};
        rret = recv(net_fd, password, t->file_length, MSG_WAITALL);
        ERROR_CHECK(rret, -1, "对端关闭");

        //判断是否是注册行为 回复客户端是否登录成功
        if(t->isRegister == 0)
        {
            //是登录行为
            //判断是否有用户名这个目录
            if(doesHaveUser(*t) == -1)
            {
                //不存在该用户
                //登录失败
                t->isLoginFailed = 1;
            }
            else
            {
                //存在
                t->isLoginFailed = 0;
            }
        }
        else
        {
            //是注册行为
            //isRegister == 1
            //创建目录
            if(doesHaveUser(*t) == 0)
            {
                int ret = createUser(*t);
                ERROR_CHECK(ret, -1, "createUser 创建目录失败");
                //注册成功
                t->isLoginFailed = 0; 
            }
            else
            {
                //注册失败（存在该用户）
                t->isLoginFailed = 1;
            }
        }
        //到这里要回复客户端是否登录成功
        //这里自定义协议里存有路径名
        rret = send(net_fd, t, sizeof(train_t), MSG_NOSIGNAL);
        ERROR_CHECK(rret, -1, "对端关闭");
        if(t->isLoginFailed == 0)
        {
            //登录成功可以退出循环
            return 0;
        }
        //如果是登录失败
        //注册成功
        //注册失败都继续循环
    }
}
