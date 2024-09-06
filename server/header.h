#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <pthread.h>
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
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <mysql/mysql.h>
#include "queue.h"

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
    //判断是否要发送正文内容
    bool send_flag;

    //控制信息的字符数组
    char control_msg[1024];
}train_t;

//服务端基本路径
#define BASE_PATH "~/files"

typedef struct pool_s
{
    //共用资源
    pthread_t *pthread_list;
    
    //线程数量
    int thread_num;

    //队列
    Queue q;

    //锁
    pthread_mutex_t lock;

    //条件变量
    pthread_cond_t cond;

    //退出标记位
    int exit_flag;
}pool_t;

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

//初始化线程池
//第一个参数是共用结构体
int initThreads(pool_t *pool);

//初始化socket
int initSocket(int *socket_fd);

//添加监听
int addEpoll(int epoll_fd, int fd);

//子线程工作函数
int doWorker(int net_fd);

//分析协议
int analysisProtocol(train_t *t, int net_fd);

//路径拼接
int pathConcat(train_t t, char *real_path);

//ls的命令
int lsCommand(train_t t, int net_fd);

//cd的命令
int cdCommand(train_t t, int net_fd);

//pwd的命令
int pwdCommand(train_t t, int net_fd);

//puts的命令
int putsCommand(train_t t, int net_fd);

//gets的命令
int getsCommand(train_t t, int net_fd);

//rm的命令
int rmCommand(train_t t, int net_fd);

#endif
