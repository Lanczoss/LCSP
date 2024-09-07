#include "header.h"

//注册动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能注册成功
int registerSystem(train_t *t, int socket_fd)
{
    //读取用户名
    char user_name[512] = {0};
    printf("Enter Username:");
    ssize_t rret = read(STDIN_FILENO, user_name, sizeof(user_name) - 1);
    ERROR_CHECK(rret, -1, "read username");
    char user_path[1024] = {0}; 
    sprintf(user_path, "%s%s", user_name, "/");
    //将路径名存入自定义协议中
    strcpy(t->control_msg, user_path);

    char password[1024] = {0};
    while(1)
    {
        bzero(password, sizeof(password));
        //读取两次密码
        printf("Enter Password:");
        fflush(stdout);
        rret = read(STDIN_FILENO, password, sizeof(password));
        ERROR_CHECK(rret, -1, "EOF");
        //对比用的密码数组
        char compare_password[1024] = {0};
        printf("Retype Password:");
        fflush(stdout);
        rret = read(STDIN_FILENO, compare_password, sizeof(compare_password));
        ERROR_CHECK(rret, -1, "EOF");
        if(strcmp(password, compare_password) == 0)
        {
            //密码正确
            break;
        }
        printf("密码两次输入不一致，请重新输入。\n");
    }
    //密码正文长度
    t->file_length = strlen(password);

    //现在是注册行为
    t->isRegister = 1;

    //先发送一次登录信息
    rret = send(socket_fd, t, sizeof(train_t), MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send login msg");

    //再发送一次密码
    rret = send(socket_fd, password, t->file_length, MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send password");

    //注册行为 接收服务器回复
    bzero(t, sizeof(train_t));
    rret = recv(socket_fd, t, sizeof(train_t), MSG_WAITALL);
    ERROR_CHECK(rret, -1, "服务器关闭");

    //将路径长度放入自定义协议中
    t->path_length = strlen(user_path);
    return 0;
}
