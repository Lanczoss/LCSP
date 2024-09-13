#include "header.h"
#include <termios.h>

void enPassword(int num)
{
    printf("\nEnter Password:");
    fflush(stdout);
}
void retypePassword(int num)
{
    printf("\nRetype Password:");
    fflush(stdout);
}
void enUsername(int num)
{
    printf("\nEnter Username:");
    fflush(stdout);
}

//注册动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能注册成功
int registerSystem(train_t *t, int *socket_fd)
{
    //读取用户名
    char user_path[512] = {0};
    while(user_path[0] == '\0' || user_path[0] == '\n')
    {
        signal(SIGINT, enUsername);
        printf("Enter Username:");
        fflush(stdout);
        ssize_t rret = read(STDIN_FILENO, user_path, sizeof(user_path) - 1);
        ERROR_CHECK(rret, -1, "read username");
    }
    user_path[strlen(user_path) - 1] = '\0';
    strcat(user_path, "/");
    //将路径名存入自定义协议中
    strcpy(t->control_msg, user_path);
    t->path_length = strlen(user_path);
    char password[1024] = {0};
    //不展示密码
    struct termios oldt, newt;

    // 获取当前终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 关闭回显功能
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while(1)
    {
        bzero(password, sizeof(password));
        while(password[0] == '\0' || password[0] == '\n')
        {
            signal(SIGINT, enPassword);
            //读取两次密码
            printf("Enter Password:");
            fflush(stdout);
            ssize_t rret = read(STDIN_FILENO, password, sizeof(password) - 1);
            ERROR_CHECK(rret, -1, "EOF");
            printf("\n");
        }
        //对比用的密码数组
        char compare_password[1024] = {0};
        while(compare_password[0] == '\0' || compare_password[0] == '\n')
        {
            signal(SIGINT, retypePassword);
            printf("Retype Password:");
            fflush(stdout);
            ssize_t rret = read(STDIN_FILENO, compare_password, sizeof(compare_password));
            ERROR_CHECK(rret, -1, "EOF");
            printf("\n");
        }
        if(strcmp(password, compare_password) == 0)
        {
            // 恢复原来的终端设置
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            printf("\n");
            //去除换行符
            password[strlen(password) - 1] = '\0';
            //密码正确
            break;
        }
        printf("密码两次输入不一致，请重新输入。\n");
    }
    //密码正文长度
    t->file_length = strlen(password);

    //现在是注册行为
    t->isRegister = 1;

    //建立socket连接
    int ret = initSocket(socket_fd);
    ERROR_CHECK(ret, -1, "initSocket");

    //先发送一次登录信息
    ssize_t rret = send(*socket_fd, t, sizeof(train_t), MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send login msg");

    //再发送一次密码
    rret = send(*socket_fd, password, t->file_length, MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send password");

    //注册行为 接收服务器回复
    bzero(t, sizeof(train_t));
    rret = recv(*socket_fd, t, sizeof(train_t), MSG_WAITALL);
    ERROR_CHECK(rret, -1, "recv");

    //将路径长度放入自定义协议中
    t->path_length = strlen(user_path);
    return 0;
}
