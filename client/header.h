#ifndef HEADER_H
#define HEADER_H

//去除epoll、pthread和mysql头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/sendfile.h>

enum
{
    FALSE,
    TRUE,
    LS,
    CD,
    PWD,
    PUTS,
    GETS,
    REMOVE,
    RM
};

//自定义协议头部信息
typedef struct train_s
{
    //枚举的头部信息
    int command;

    //本次发送记录路径的长度
    int msg_length;

    //本次发送内容/正文的长度
    int file_length;

    //标记位
    //记录本次发送有几个参数
    int parameter_num;

    //标记位
    //是否是本端要发送文件
    //0是本端不发文件
    //1是本端要发文件
    bool send_flag;

    //控制信息的字符数组
    char control_msg[1024];
}train_t;

// 检查命令行参数数量是否符合预期
#define ARGS_CHECK(argc, expected) \
    do { \
        if ((argc) != (expected)) { \
            fprintf(stderr, "args num error!\n"); \
            return -1; \
        } \
    } while (0)

// 检查返回值是否是错误标记,若是则打印msg和错误信息
#define ERROR_CHECK(ret, error_flag, msg) \
    do { \
        if ((ret) == (error_flag)) { \
            perror(msg); \
            return -1; \
        } \
    } while (0)

#define THREAD_ERROR_CHECK(ret, msg) \
    do{ \
        if((ret) != 0) { \
            fprintf(stderr, "%s:%s\n", msg, strerror(ret)); \
        } \
    } while(0)

//读配置文件
int getParameter(void *key, void *value);

//初始化socket
int initSocket(int *socket_fd);

//分析协议
int analysisProtocol(train_t t, int socket_fd);

//拆分字符串
int splitCommand(train_t *t, char *buf);

//接收服务端发送的文件内容
int recvFile(int socket_fd);

//ls的命令
int lsCommand(train_t t, int socket_fd);

//cd的命令
int cdCommand(train_t t, int socket_fd);

//pwd的命令
int pwdCommand(train_t t, int socket_fd);

//puts的命令
int putsCommand(train_t t, int socket_fd);

//gets的命令
int getsCommand(train_t t, int socket_fd);

//rm的命令
int rmCommand(train_t t, int socket_fd);
#endif
