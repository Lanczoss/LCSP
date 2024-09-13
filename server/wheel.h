#ifndef WHEEL_H
#define WHEEL_H

#include <time.h>
// 时间轮的槽大小，表示每个槽代表的时间（秒）
#define TIME_WHEEL_SIZE 60

// 定时器结构体
typedef struct Timer {
    int fd;                // 需要管理的文件描述符
    time_t expire_time;    // 定时器过期时间
    struct Timer* next;    // 指向下一个定时器的指针
} Timer;

// 时间轮结构体
typedef struct TimeWheel {
    Timer* slots[TIME_WHEEL_SIZE]; // 存储定时器的槽数组
    int current_slot;              // 当前时间轮的槽位
} TimeWheel;

// 初始化时间轮
void initTimeWheel(TimeWheel* wheel);

// 插入定时器到时间轮
void addTimer(TimeWheel* wheel, int fd, time_t expire_time);

// 删除定时器
void removeTimer(TimeWheel* wheel, int fd);

// 处理超时事件
void handleTimeout(TimeWheel* wheel);

// 轮转时间轮
void rotateTimeWheel(TimeWheel* wheel);

// 将新的 net_fd 添加到 net_fd_arr 数组中
void addNetFd(int new_fd, int *net_fd_arr, int max_fds);

#endif //WHEEL_H
