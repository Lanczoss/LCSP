#include "header.h"

//客户端的用户操作界面
//录入用户第一次操作时的自定义协议
//并将初始用户名加入到路径名中
int interface(train_t *t, int socket_fd)
{
    int ret;
    //获取选项
    while(1)
    {
        LOG_INFO("展示登录界面");
        bzero(t, sizeof(train_t));
        t->isLoginFailed = 1;
        printf("(Y) 登录  (R) 注册  (E)退出 > ");
        fflush(stdout);
        char option[512] = {0};
        ssize_t rret = read(STDIN_FILENO, option, sizeof(option) - 1);
        ERROR_CHECK(rret, -1, "read");
        switch(option[0])
        {
        case 'y':
        case 'Y':
            LOG_INFO("用户登录");
            ret = loginSystem(t, socket_fd);
            if(ret != 0)
            {
                return -1;
            }
            if(t->isLoginFailed == 1)
            {
                //登录失败再来一次
                printf("用户名或密码不正确，请重新输入。\n\n");
                continue;
            }
            printf("登录成功\n\n");
            //打印系统信息
            struct utsname sys_msg;
            ret = uname(&sys_msg);
            ERROR_CHECK(ret, -1, "uname");
            printf("\n\n%s %s %s %s %s\n\n", 
                sys_msg.sysname,
                sys_msg.nodename,
                sys_msg.release,
                sys_msg.version,
                sys_msg.machine
            );
            //打印登录选项
            printf("Welcome to Cloud Storage Service!\n\n");
            return 0;
        case 'r':
        case 'R':
            LOG_INFO("用户注册");
            ret = registerSystem(t, socket_fd);
            if(ret != 0)
            {
                return -1;
            }
            if(t->isLoginFailed == 1)
            {
                //注册失败
                printf("该用户名已存在。\n\n");
                continue;
            }
            //注册成功也需要继续循环
            printf("注册成功\n\n");
            t->isRegister = 0;
            continue;
        case 'e':
        case 'E':
            //退出
            printf("网盘正在退出……\n\n");
            return -1;
        default:
            printf("选项输入错误，请重试\n");
        }
    }
    return 0;
}
