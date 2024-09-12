#include "header.h"

//登录/注册动作函数
//第一版第二版
//将从客户端发来的用户名和密码进行验证
//如果是登录行为，密码输入不正确，要求重试
//如果是注册行为，用户名已经存在，则失败
int loginRegisterSystem(train_t *t, int net_fd, MYSQL *mysql)
{
    while(1)
    {
        bzero(t, sizeof(train_t));
        //先接收一次的登录信息或者注册信息;
        ssize_t rret = recv(net_fd, t, sizeof(train_t), MSG_WAITALL);
        ERROR_CHECK(rret, -1, "recv");
        if(rret == 0)
        {
            return -1;
        }
        //接收密码
        char password[1024] = {0};
        rret = recv(net_fd, password, t->file_length, MSG_WAITALL);
        ERROR_CHECK(rret, -1, "recv");
        if(rret == 0)
        {
            return -1;
        }

        char user_name[1024] = {0};
        //这里清除如zs/后面的斜杆
        strncpy(user_name, t->control_msg, t->path_length - 1);
        //printf("user_name = %s\n", user_name);

        //判断是否是注册行为 回复客户端是否登录成功
        if(t->isRegister == 0)
        {
            //是登录行为
            //判断是否有用户名这个目录
            int ret = checkUserMsg(user_name, mysql);
            if(ret == 0)
            {
                //不存在该用户
                //登录失败
                t->isLoginFailed = 1;
            }
            else
            {
                //存在
                //开始验证密码
                ret = checkPassword(user_name, password, mysql);
                if(ret == -1)
                {
                    //密码错误
                    t->isLoginFailed = 1;
                }
                else
                {
                    //密码正确
                    //获取用户id
                    int uid = getUidMysql(user_name, mysql);
                    if(uid == -1)
                    {
                        t->isLoginFailed = 1;
                        return -1;
                    }
                    printf("%s的user_id = %d\n", user_name, uid);

                    //根据uid获取加密token并保存
                    bzero(t->token, sizeof(t->token));
                    enCodeToken(uid, t->token);
                    printf("%s的token = %s\n", user_name, t->token);

                    bzero(t->control_msg, sizeof(t->control_msg));
                    strcpy(t->control_msg, "/");
                    t->path_length = 1;
                    t->isLoginFailed = 0;
                }
            }
        }
        else
        {
            //是注册行为
            //isRegister == 1
            //在users表上创建条目
            int ret = checkUserMsg(user_name, mysql);
            if(ret == 0)
            {
                //开始注册
                ret = registerInsertMysql(user_name, password, mysql);
                ERROR_CHECK(ret, -1, "Register Msg insert into MySQL failed.");
                LOG_INFO("One client register success");
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
        //这里如果登录成功自定义协议里存有一个'/'和用户id
        rret = send(net_fd, t, sizeof(train_t), MSG_NOSIGNAL);
        ERROR_CHECK(rret, -1, "send");
        if(t->isLoginFailed == 0 && t->isRegister == 0 && t->token[0] != '\0')
        {
            //登录成功可以退出循环
            return 0;
        }
        //如果是登录失败
        //注册成功
        //注册失败都继续循环
    }
}
