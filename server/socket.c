#include "header.h"

/* Usage:  */
// 用于初始化socket的函数
// 参数：需要传入socket_fd对象
int initSocket(int *socket_fd)
{
    
    // 创建socket对象
    *socket_fd =socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(*socket_fd, -1, "socket");

    // 断线重连
    int reuse =   1;
    int ret_set = setsockopt(*socket_fd, SOCK_STREAM, SO_REUSEADDR, &reuse, sizeof(reuse)); 
    ERROR_CHECK(ret_set, -1, "setsockopt");

    // 调用getparameter获得ip和端口
    char ip[100] = { 0 };
    bzero(ip, sizeof(ip));
    getParameter((void *)"ip", (void *) ip);

    char port[100] = { 0 };
    bzero(port, sizeof(port));
    getParameter((void *)"port", (void *)port);

    // 创建struct socket结构体
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(port));
    sockaddr.sin_addr.s_addr = inet_addr(ip);

    // 绑定端口
    int ret_bind = bind(*socket_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    ERROR_CHECK(ret_bind, -1, "bind");

    // 监听端口
    int res_lis = listen(*socket_fd, 10);
    ERROR_CHECK(res_lis, -1, "listen");

    return 0;
}
