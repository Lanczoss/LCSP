#ifndef WHEEL_H
#define WHEEL_H

#include <time.h>
// 时间轮的槽大小，表示每个槽代表的时间（秒）
#define TIME_WHEEL_SIZE 30

// 链表结点结构体
typedef struct timer_s {
    int fd;
    int alive;            
    struct timer_s* next;
} Timer;

// 循环指针数组结构体
typedef struct TimeWheel {
    Timer* slots[TIME_WHEEL_SIZE]; // 存储定时器的槽数组
} TimeWheel;

// 初始化时间轮
void initTimeWheel(TimeWheel* wheel);

// 插入定时器到时间轮
int addTimer(TimeWheel* wheel, int fd);

//连接关闭从链表中移除
int removeTimer(TimeWheel* wheel, int fd);

// 处理超时事件
void handleTimeout(TimeWheel* wheel);

// 将新的 net_fd 添加到 net_fd_arr 数组中
int addNetFd(TimeWheel* wheel, int *net_fd_arr, int length);

#endif //WHEEL_H
