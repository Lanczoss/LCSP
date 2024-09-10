#include "header.h"

//获取mysql数据库用的现在的datetime
//需要传入一个指向buf的空间
//返回值为指向buf空间的指针
char *getNowTimeMysql(char *buf)
{
    // 获取当前时间
    time_t t;
    struct tm *tm;
    time(&t);
    tm = localtime(&t);
    // 将当前时间格式化为 DATETIME 格式
    sprintf(buf, "%d-%d-%d %02d:%02d:%02d",
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec
        );
    printf("现在的datetime为 %s\n", buf);
    return buf;
}