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
#include <errno.h>

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
    EXIT,
    RENAME
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

    //用户id，存token值
    char uid[512];

    // 异常标志位
    int error_flag;

    // 当前用户所在目录层数，初始为0
    int current_layers;

    //控制信息的字符数组
    char control_msg[1024];
}train_t;

extern FILE *log_info_file;
extern FILE *log_error_file;
// 定义日志级别的宏
// 使用示例:LOG_INFO("正确信息");打印到info.log
// 例如打开文件：ERROR_CHECK(file_fd, -1, "open"),  记录到error.log
// LOG_MYSQL_ERROR(mysql);  检查mysql 记录到error.log
// CHECK_MYSQL_RESULT(result);  检查 MYSQL_RES 是否为 NULL 并记录错误 记录到error.log
// CHECK_NUM_ROWS(rows);    检查行数是否为 0 并记录信息 记录到error.log
// 日志级别宏
#define LOG_INFO(message) \
    do { \
        if(log_info_file == NULL){ \
            log_info_file = fopen("info.log", "a"); \
            if(log_info_file == NULL){ \
                perror("无法打开 info.log"); \
            } \
        } \
        if(log_info_file != NULL){ \
            writeLog(log_info_file, "INFO", __FILE__, __LINE__, message); \
        } \
    } while(0)

#define LOG_ERROR(message) \
    do { \
        if(log_error_file == NULL){ \
            log_error_file = fopen("error.log", "a"); \
            if(log_error_file == NULL){ \
                perror("无法打开 error.log"); \
            } \
        } \
        if(log_error_file != NULL){ \
            writeLog(log_error_file, "ERROR", __FILE__, __LINE__, message); \
        } \
    } while(0)

#define LOG_PERROR(message) \
    do { \
        char error_msg[256]; \
        snprintf(error_msg, sizeof(error_msg), "%s: %s", message, strerror(errno)); \
        LOG_ERROR(error_msg); \
    } while(0)
// 定义LOG_MYSQL_ERROR宏，专门用于检测和记录MySQL的错误
// 使用strlen(mysql_err) > 0是为了避免日志中出现大量的空字符
// mysql_error在没有出错时返回空字符
// 定义LOG_MYSQL_ERROR宏，专门用于检测和记录MySQL的错误
#define LOG_MYSQL_ERROR(mysql) \
    do { \
        const char *mysql_err = mysql_error(mysql); \
        if (mysql_err && strlen(mysql_err) > 0) { \
            char error_msg[512]; \
            snprintf(error_msg, sizeof(error_msg), "MySQL Error: %s", mysql_err); \
            LOG_ERROR(error_msg); \
        } \
    } while (0)

// 检查 MYSQL_RES 是否为 NULL 并记录错误
#define CHECK_MYSQL_RESULT(result) \
    do { \
        if ((result) == NULL) { \
            LOG_ERROR("mysql_store_result() 返回 NULL"); \
            LOG_ERROR(mysql_error(&mysql)); \
        } \
    } while (0)

// 检查行数是否为 0 并记录信息
#define CHECK_NUM_ROWS(num_rows) \
    do { \
        if ((num_rows) == 0) { \
            LOG_ERROR("未找到任何行。"); \
        } else { \
            printf("行数: %lu\n", (unsigned long)(num_rows)); \
        } \
    } while (0)
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
            LOG_PERROR(msg); \
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
int rmCommand(train_t t, int socket_fd);

//mkdir的命令
int mkdirCommand(train_t *t, int socket_fd);

//重命名文件
int reName(train_t t, int socket_fd);

//客户端的用户操作界面
//录入用户第一次操作时的自定义协议
//并将初始用户名加入到路径名中
int interface(train_t *t, int *socket_fd);

//登录动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能登录成功
int loginSystem(train_t *t, int *socket_fd);

//注册动作函数
//第一版第二版
//将用户名和密码发送至服务器处接收
//这里密码输入什么都能注册成功
int registerSystem(train_t *t, int *socket_fd);

// 日志记录函数
// 第一个参数：日志文件指针，用于指定日志文件(log_info_file 或 log_error_file)
// 第二个参数，日志级别(如"INFO"或"ERROR")
// 第三个参数，源代码文件名,通常通过'__FILE__'宏传递
// 第四个参数：源代码行号，通常通过'__LINE__'宏传递
// 第五个参数：日志消息,记录的具体内容
int writeLog(FILE *log_file, const char *level, const char *file, int line, const char *message);

// 日志关闭函数声明，确保在程序结束时关闭日志文件
void closeLog();
#endif
