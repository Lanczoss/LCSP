#include "header.h"

/* Usage:  */
//初始化socket                  
int initSocket(int *socket_fd)
{
    // 初始化socket对象
    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(*socket_fd, -1, "socket");

    // 断线重连
    int reuse = 1;
    int ret_set = setsockopt(*socket_fd, SOCK_STREAM, SO_REUSEADDR, &reuse, sizeof(reuse));
    ERROR_CHECK(ret_set, -1, "setsockopt");

    // 从getParameter获取ip和端口
    char ip[100];
    memset(ip, 0, sizeof(ip));  
    getParameter((void *)"ip", (void *)ip);

    char port[100];
    memset(port, 0, sizeof(port));
    getParameter((void *)"port", (void *)port);
    
    // 初始化struct socket 结构体
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(port));
    sockaddr.sin_addr.s_addr = inet_addr(ip);

    // 绑定端口
    int ret_bind = bind(*socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    ERROR_CHECK(ret_bind, -1, "bind");

    // 监听端口
    int ret_lis = listen(*socket_fd, 10);
    ERROR_CHECK(ret_lis, -1, "listen");


    return 0;
}

