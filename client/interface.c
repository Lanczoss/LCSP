#include "header.h"

//客户端的用户操作界面
//录入用户第一次操作时的自定义协议
//并将初始用户名加入到路径名中
int interface(train_t *t, int socket_fd)
{
    //打印系统信息
    struct utsname sys_msg;
    int ret = uname(&sys_msg);
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
    printf("(Y) 登录  (R) 注册  (E)退出\n");
    //获取选项
    while(1)
    {
        printf(">");
        fflush(stdout);
        char option[512] = {0};
        ssize_t rret = read(STDIN_FILENO, option, sizeof(option) - 1);
        ERROR_CHECK(rret, -1, "read");
        switch(option[0])
        {
        case 'y':
        case 'Y':
            //登录
           ret = loginSystem(t, socket_fd);
           ERROR_CHECK(ret, -1, "loginSystem");
           break;
        case 'r':
        case 'R':
           //注册先只做登录
           //ret = registerSystem(t, socket_fd);
           //ERROR_CHECK(ret, -1, "registerSystem");
           break;
        case 'e':
        case 'E':
           //退出
           break;
        default:
           printf("选项输入错误，请重试\n");
        }
    }
    return 0;
}
