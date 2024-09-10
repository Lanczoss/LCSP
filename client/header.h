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
#include <sys/utsname.h>
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
#include <sys/utsname.h>

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
    MKDIR,
    NORMAL,
    ABNORMAL,
};

//自定义协议头部信息
typedef struct train_s
{
    //枚举的头部信息
    int command;

    //本次发送记录路径名的长度
    int path_length;

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

    //标记位
    //用于判断第一次交互时是不是注册行为
    //默认0，1代表本次是注册行为
    bool isRegister;

    //标记位
    //用于标记用户是否登录失败
    //默认0登录成功，1代表登录失败
    bool isLoginFailed;

    //用户id
    int uid;

    // 异常标志位
    int error_flag;

    // 当前用户所在目录层数，初始为0
    int current_layers;

    //控制信息的字符数组
    char control_msg[1024];
}train_t;

// 定义日志级别的宏，简化日志记录的使用
// __FILE__ 用于捕获源文件名，__LINE__ 用于捕获代码所在行
#define LOG_INFO(message)  writeLog("INFO", __FILE__, __LINE__, message)
#define LOG_ERROR(message) writeLog("ERROR", __FILE__, __LINE__, message)
#define LOG_WARN(message)  writeLog("WARNING", __FILE__, __LINE__, message)

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
            LOG_ERROR(msg); \
            return -1; \
        } \
    } while (0)

#define THREAD_ERROR_CHECK(ret, msg) \
    do{ \
        if((ret) != 0) { \
            fprintf(stderr, "%s:%s\n", msg, strerror(ret)); \
        } \
    } while(0)

//保存从服务器发来的用户实际路径
#define USER_PATH "./";

//读配置文件
int getParameter(void *key, void *value);

//初始化socket
int initSocket(int *socket_fd);

//分析协议
int analysisProtocol(train_t *t, int socket_fd);

//拆分字符串
int splitCommand(train_t *t, char *buf);

//接收服务端发送的文件内容
int recvFile(int socket_fd);

//ls的命令
int lsCommand(train_t t, int socket_fd);

//cd的命令
int cdCommand(train_t *t, int socket_fd);

//pwd的命令
int pwdCommand(train_t t);

//puts的命令
int putsCommand(train_t t, int socket_fd);

//gets的命令
int getsCommand(train_t t, int socket_fd);

//rm的命令
int rmCommand(train_t *t, int socket_fd);

//mkdir的命令
int mkdirCommand(train_t *t, int socket_fd);

//客户端的用户操作界面
//录入用户第一次操作时的自定义协议
//并将初始用户名加入到路径名中
int interface(train_t *t, int socket_fd);

//登录动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能登录成功
int loginSystem(train_t *t, int socket_fd);

//注册动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能注册成功
int registerSystem(train_t *t, int socket_fd);

// 日志记录
// 第一个参数，日志级别
// 第二个参数，源代码文件名
// 第三个参数：源代码行号 
// 第四个参数：日志消息
/* usage: 4writeLog("client_log", "输入错误");*/
void writeLog(const char * level,const char *file, int line,const char * message);

// 日志关闭函数声明，确保在程序结束时关闭日志文件
void closeLog();
#endif
