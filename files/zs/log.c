 第二个参数，源代码文件名
// 第三个参数：源代码行号 
// 第四个参数：日志消息
/* usage: 4writeLog("client_log", "输入错误");*/
void writeLog(const char * level,const char *file, int line,const char * message)
{
        if(log_file == NULL)
        {
            // TODO：打开日志文件记得关闭
            // 默认打开日志文件，只在第一次打开
            log_file = fopen("client_log.txt", "a");
            if(log_file == NULL)
            {
                perror("无法打开文件");
                return;
            }
        }
        // 获取当前时间
        char time_str[26];
        getCurrentTime(time_str, sizeof(time_str));

        // 获取主机名
        char hostname[64];
        getHostname(hostname, sizeof(hostname));

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
void closeLog()
{
    if(log_file != NULL)
    {
        fclose(log_file);
    }
}
