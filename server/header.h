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
#include <sys/utsname.h>    // uname()需要用到的头文件
#include <errno.h>
#include <crypt.h>

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
    ABNORMAL,
    NORMAL,
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

    //用户id
    int uid;

    //错误标志
    int error_flag;
    
    //当前层数
    int current_layers;

    //控制信息的字符数组
    char control_msg[1024];
}train_t;

//服务端基本路径
#define BASE_PATH "../files/"

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

    // 错误标志位
    int error_flag;
    
    // 当前层数
    int current_layers;
}pool_t;

extern FILE *log_info_file;
extern FILE *log_error_file;
// 定义日志级别的宏
// 使用示例:LOG_INFO("正确信息");  LOG_PERROR("错误信息"); LOG_MYSQL_ERROR(mysql);
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
            LOG_PERROR(msg); \
        } \
    } while(0)

//创建目录files或者判断是否有files
int createBaseFiles(void);

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
int doWorker(MYSQL *mysql, int net_fd);

//分析协议
int analysisProtocol(train_t t, int net_fd, MYSQL *mysql);

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
int cdCommand(train_t t, int net_fd, MYSQL *sql);

//pwd的命令
int pwdCommand(train_t t, int net_fd);

//puts的命令
int putsCommand(train_t t, int net_fd);

//gets的命令
int getsCommand(train_t t, int net_fd, MYSQL *sql);

//rm的命令
int rmCommand(train_t t, int net_fd,MYSQL*mysql);

//创建文件夹
int mkdirCommand(train_t t, int net_fd, MYSQL *mysql);

// 子线程的入口函数
void *threadMain(void *p);

// 日志记录函数
// 第一个参数：日志文件指针，用于指定日志文件(log_info_file 或 log_error_file)
// 第二个参数，日志级别(如"INFO"或"ERROR")
// 第三个参数，源代码文件名,通常通过'__FILE__'宏传递
// 第四个参数：源代码行号，通常通过'__LINE__'宏传递
// 第五个参数：日志消息,记录的具体内容
int writeLog(FILE *log_file, const char *level, const char *file, int line, const char *message);

// 日志关闭函数声明，确保在程序结束时关闭日志文件
void closeLog();

//根据路径名验证数据库中用户信息
int checkUserMsg(const char *user_name, MYSQL *mysql);

//登录/注册动作函数
//第三版
//将从客户端发来的用户名和密码进行验证
//如果是登录行为，密码输入不正确，要求重试
//如果是注册行为，用户名已经存在，则失败
int loginRegisterSystem(train_t *t,  int net_fd, MYSQL *mysql);

//子线程连接数据库函数
int connectMysql(MYSQL  **mysql);

//验证密码
//需要查找对应用户的盐值来获取hash值
int checkPassword(const char *user_name, const char *password, MYSQL *mysql);

//将得到的用户名和密码插入到users表中
//密码需要盐值加密
//密码还需要hash值
int registerInsertMysql(const char *user_name, const char *password, MYSQL *mysql);

//定义盐值长度和sha512标识
#define SALT_PREFIX "$6$"
#define SALT_LENGTH 8
//获取散列的函数
//需要传入一个指向buf的空间
//一个盐值
//一个明文密码
int getHashValue(char *buf, char *salt, const char *password);

//根据用户名从数据库中获取uid的函数
int getUidMysql(const char *user_name, MYSQL *mysql);
//创建文件夹目录
int insertDir(train_t t, char * real_path, char* filename,MYSQL*mysql);
//删除文件
int deleteFile(train_t t, char * file_path, MYSQL*mysql);

int getFileId(train_t t, MYSQL * mysql);
#endif
