#include "header.h"

//登录动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能登录成功
int loginSystem(train_t *t, int socket_fd)
{
    //读取用户名
    char user_path[1024] = {0};
    printf("Enter Username:");
    fflush(stdout);
    ssize_t rret = read(STDIN_FILENO, user_path, sizeof(user_path) - 1);
    ERROR_CHECK(rret, -1, "read username");
    user_path[strlen(user_path) - 1] = '\0';
    strcat(user_path, "/");
    //将路径名存入自定义协议中
    strcpy(t->control_msg, user_path);
    t->path_length = strlen(user_path);

    //读取密码
    char password[1024] = {0};
    printf("Enter Password:");
    fflush(stdout);
    rret = read(STDIN_FILENO, password, sizeof(password));
    ERROR_CHECK(rret, -1, "read password");
    //密码正文长度
    t->file_length = strlen(password);

    //先发送一次登录信息
    rret = send(socket_fd, t, sizeof(train_t), MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send login msg");

    //再发送一次密码
    rret = send(socket_fd, password, t->file_length, MSG_NOSIGNAL);
    ERROR_CHECK(rret, -1, "send password");

    //登录行为 接收服务器回复
    bzero(t, sizeof(train_t));
    rret = recv(socket_fd, t, sizeof(train_t), MSG_WAITALL);
    ERROR_CHECK(rret, -1, "服务器关闭");

    //将路径长度放入自定义协议中
    t->path_length = strlen(user_path);
    return 0;
}

