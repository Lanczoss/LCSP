#include "header.h"

void printInterface(void)
{
    int color_code = 160; // 起始颜色代码
    int step = 1;         // 每次颜色变化的步长

    printf("\033[5m");
    // 打印每一行时，颜色代码逐渐增加以形成渐变效果
    printf("\033[38;5;%dm ________  ___       ________  ___  ___  ________     \033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm|\\   ____\\|\\  \\     |\\   __  \\|\\  \\|\\  \\|\\   ___ \\    \033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm\\ \\  \\___|\\ \\  \\    \\ \\  \\|\\  \\ \\  \\ \\  \\ \\  \\_\\ \\   \033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm \\ \\  \\    \\ \\  \\    \\ \\  \\ \\  \\ \\  \\ \\  \\ \\  \\ \\ \\  \033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm  \\ \\  \\____\\ \\  \\____\\ \\  \\ \\  \\ \\  \\ \\  \\ \\  \\_\\ \\ \033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm   \\ \\_______\\ \\_______\\ \\_______\\ \\_______\\ \\_______\\\033[0m\n", color_code);
    color_code += step;
    printf("\033[5m");
    printf("\033[38;5;%dm    \\|_______|\\|_______|\\|_______|\\|_______|\\|_______|\033[0m\n", color_code);
    printf("                                                       \n");
    
    //打印系统信息
    struct utsname sys_msg;
    uname(&sys_msg);
    printf("\n\n%s %s %s %s %s\n\n", 
        sys_msg.sysname,
        sys_msg.nodename,
        sys_msg.release,
        sys_msg.version,
        sys_msg.machine
    );
    //打印登录选项
    printf("Welcome to Cloud Storage Service!\n\n");
    printf("\033[38;5;135m输入 'exit'('e') 或 'quit'('q') 退出网盘.\033[0m \n\n");
    printf("\xF0\x9F\xA6\x84 \xF0\x9F\x8C\x88 \xE2\x9D\xA4\xEF\xB8\x8F \xF0\x9F\xA7\xA1 \xF0\x9F\x92\x9B \xF0\x9F\x92\x9A \xF0\x9F\x92\x99 \xF0\x9F\x92\x9C \xE2\x9C\xA8\n \n");
}

void loginFunc(int num)
{
    printf("\n\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m(Y) 登录  (R) 注册  (E)退出 > ");
    fflush(stdout);
}

//客户端的用户操作界面
//录入用户第一次操作时的自定义协议
//并将初始用户名加入到路径名中
int interface(train_t *t, int socket_fd)
{
    printf("\xF0\x9F\x91\xBB\xF0\x9F\x91\xBB 欢迎使用 \xF0\x9F\x91\xBB\xF0\x9F\x91\xBB\n");
    int ret;
    //获取选项
    while(1)
    {
        signal(SIGINT, loginFunc);
        LOG_INFO("Show login options.");
        bzero(t, sizeof(train_t));
        t->isLoginFailed = 1;
        printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m(Y) 登录  (R) 注册  (E)退出 > ");
        fflush(stdout);
        char option[512] = {0};
        ssize_t rret = read(STDIN_FILENO, option, sizeof(option) - 1);
        ERROR_CHECK(rret, -1, "read");
        switch(option[0])
        {
        case 'y':
        case 'Y':
            LOG_INFO("Login");
            ret = loginSystem(t, socket_fd);
            if(ret != 0)
            {
                return -1;
            }
            if(t->isLoginFailed == 1)
            {
                //登录失败再来一次
                printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m用户名或密码不正确，请重新输入。\n\n");
                continue;
            }
            printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m登录成功\n\n");
            //打印用户界面
            printInterface();
            return 0;
        case 'r':
        case 'R':
            LOG_INFO("Register");
            ret = registerSystem(t, socket_fd);
            if(ret != 0)
            {
                return -1;
            }
            if(t->isLoginFailed == 1)
            {
                //注册失败
                printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m该用户名已存在。\n\n");
                continue;
            }
            //注册成功也需要继续循环
            printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m注册成功\n\n");
            t->isRegister = 0;
            continue;
        case 'e':
        case 'E':
            //退出
            printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m网盘正在退出……\n\n");
            return -1;
        default:
            printf("\033[38;5;208m[\xE2\x98\x81 \033[38;5;208m ] \033[0m选项输入错误，请重试\n");
        }
    }
    return 0;
}
