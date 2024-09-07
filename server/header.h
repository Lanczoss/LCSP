#ifndef HEADER_H
#define HEADER_H

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
#include "command.h"
#include <sys/utsname.h>    // uname()需要用到的头文件

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
    RM,
    MKDIR
};

//自定义协议头部信息
typedef struct train_s
{
    //枚举的头部信息
    int command;

    //本次发送记录路径的长度
    int path_length;

    //本次发送内容/正文的长度
    int file_length;

    //标记位
    //记录本次发送有几个参数
    int parameter_num;

    //标记位
    //判断是否要发送正文内容
    bool send_flag;

    //标记位
    //用于判断第一次交互时是不是注册行为
    //默认0，1代表本次是注册行为
    bool isRegister;

    //标记位
    //用于标记用户是否登录失败
    //默认0登录成功，1代表登录失败
    bool isLoginFailed;

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

// 定义日志级别的宏
#define LOG_INFO(message) writeLog("INFO", __FILE__,__LINE__,message)
#define LOG_ERROR(message) writeLog("ERROR", __FILE__, __LINE__, message)
#define LOG_WARN(message) writeLog("WARNING", __FILE__, __LINE__, message)

// 检查命令行参数数量是否符合预期
#define ARGS_CHECK(argc, expected) \
    do { \
        if ((argc) != (expected)) { \
            fprintf(stderr, "args num error!\n"); \
            return -1; \
        } \
    } while (0)

// 检查返回值是否是错误标记,若是则打印msg和错误信息
// 将错误信息打印到日志文件中
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

//从train自定义协议中取得参数
//第一个参数指自定义协议
//第二个参数指需要第几个参数
//第三个参数指字符数组缓冲区
int splitParameter(train_t t, int num, char *buf);

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

// 子线程的入口函数
void *threadMain(void *p);

// 日志记录
// 第一个参数，日志级别
// 第二个参数，源代码文件名
// 第三个参数：源代码行号 
// 第四个参数：日志消息
/* usage: 4writeLog("client_log", "输入错误");*/
void writeLog(const char * level,const char *file, int line,const char * message);

// 日志关闭函数声明，确保在程序结束时关闭日志文件
void closeLog();
//
//根据路径名判断是否有用户名这个目录
int doesHaveUser(train_t t);

//登录/注册动作函数
//第一版第二版
//将从客户端发来的用户名和密码进行验证
//如果是登录行为，密码输入不正确，要求重试
//如果是注册行为，用户名已经存在，则失败
int loginRegisterSystem(train_t *t,  int net_fd);

//注册时尝试创建用户根目录的函数
//第一版先遍历目录
int createUser(train_t t);

#endif
