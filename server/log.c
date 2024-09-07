#include "header.h"

/* Usage:  */

// 静态变量存储日志文件指针，确保文件只打开一次
static FILE *log_file = NULL;

// 获取主机名
// 第一个参数：获取主机名
// 第二个参数：主机名的大小
void get_hostname(char *hostname, size_t size)
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
void get_current_time(char *buffer, size_t size)
{
    // uname的结构体
    // 当前的时间
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S ", tm_info);
}

// 日志记录
// 第一个参数，日志级别
// 第二个参数，源代码文件名
// 第三个参数：源代码行号 
// 第四个参数：日志消息
/* usage: 4writeLog("client_log", "输入错误");*/
void writeLog(const char * level,const char *file, int line,const char * message)
{
        if(log_file == NULL)
        {
            // TODO：打开日志文件记得关闭
            // 默认打开日志文件，只在第一次打开
            log_file = fopen("system_log.txt", "a");
            if(log_file == NULL)
            {
                perror("无法打开文件");
                return;
            }
        }
        // 获取当前时间
        char time_str[26];
        get_current_time(time_str, sizeof(time_str));

        // 获取主机名
        char hostname[64];
        get_hostname(hostname, sizeof(hostname));

        // 获取当前进程ID和进程名
        pid_t pid = getpid();
        // TODO:根据实际进程名替换
        char process_name[] = "file_server";
        
        fprintf(log_file, "%s %s %s[%d]: %s: %s (in %s:%d)\n",
                time_str,   // 时间
                hostname,   // 主机名
                process_name,   // 进程名称
                pid,            // 进程ID
                level,          // 日志级别
                message,        // 日志消息
                file,           // 源代码文件名
                line            // 源代码行号    
                );

        fflush(log_file);   // 刷新缓冲区，将日志写入文件
}

// 在程序结束时，关闭日志文件
void close_log()
{
    if(log_file != NULL)
    {
        fclose(log_file);
    }
}
