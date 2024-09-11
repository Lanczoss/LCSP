#include "header.h"

// 全局变量初始化
FILE *log_info_file = NULL;
FILE *log_error_file = NULL;

// 获取主机名
// 第一个参数：获取主机名
// 第二个参数：主机名的大小
void getHostname(char *hostname, size_t size)
{
    // 系统调用函数uname()需要用到的结构体 uname()函数是用于返回系统的信息
    struct utsname sys_info;
    // uname:0代表成功 -1代表失败
    if(uname(&sys_info) == 0)
    {
        strncpy(hostname, sys_info.nodename, size);
    }else{
        strncpy(hostname, "unknown", size);
    }
}

// 获取当前时间的格式化字符串
// 第一个参数：存储当前字符串的数组
// 第二个参数：第一个参数的大小
void getCurrentTime(char *buffer, size_t size)                
{
    // uname的结构体
    // 当前的时间
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S ", tm_info);
}

// 日志记录函数
// 第一个参数：日志文件指针，用于指定日志文件（log_info_file 或 log_error_file）
// 第二个参数：日志级别（如 "INFO" 或 "ERROR"）
// 第三个参数：源代码文件名，通常通过 `__FILE__` 宏传递
// 第四个参数：源代码行号，通常通过 `__LINE__` 宏传递
// 第五个参数：日志消息，记录的具体内容
int writeLog(FILE *log_file, const char *level, const char *file, int line, const char *message) {
    // 存储当前时间的字符串
    char time_str[26];
    getCurrentTime(time_str, sizeof(time_str));  // 获取当前时间的格式化字符串

    // 存储主机名
    char hostname[64];
    getHostname(hostname, sizeof(hostname));  // 获取主机名

    // 获取当前进程ID
    pid_t pid = getpid();
    char process_name[] = "client_server";  // 进程名称，可以根据需要进行修改

    // 将日志记录写入日志文件
    fprintf(log_file, "%s %s %s[%d]: %s: %s (in %s:%d)\n",
            time_str,      // 当前时间
            hostname,      // 主机名
            process_name,  // 进程名称
            pid,           // 进程ID
            level,         // 日志级别
            message,       // 日志消息
            file,          // 源代码文件名
            line           // 源代码行号
    );
    
    fflush(log_file);  // 刷新缓冲区，确保日志立即写入文件

    return 0;
}

// 关闭日志文件的函数
// 关闭日志文件（log_info_file 和 log_error_file），在程序结束时调用以释放文件资源
void closeLog() {
    // 如果信息日志文件已打开，则关闭它
    if (log_info_file != NULL) {
        fclose(log_info_file);
    }
    
    // 如果错误日志文件已打开，则关闭它
    if (log_error_file != NULL) {
        fclose(log_error_file);
    }
}


