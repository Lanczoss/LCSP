#include "wheel.h"

// 初始化时间轮
void initTimeWheel(TimeWheel* wheel) {
    // 将时间轮的每个槽初始化为空
    for (int i = 0; i < TIME_WHEEL_SIZE; i++) {
        wheel->slots[i] = NULL;
    }
    // 将当前槽位设置为0
    wheel->current_slot = 0;
}

// 插入定时器到时间轮
void addTimer(TimeWheel* wheel, int fd, time_t expire_time) {
    // 计算定时器应插入的槽位
    int slot = (wheel->current_slot + (expire_time - time(NULL)) % TIME_WHEEL_SIZE) % TIME_WHEEL_SIZE;

    // 创建一个新的定时器
    Timer* timer = (Timer*)malloc(sizeof(Timer));
    timer->fd = fd;
    timer->expire_time = expire_time;
    // 将定时器插入到计算出的槽位中，放在链表的开头
    timer->next = wheel->slots[slot];
    wheel->slots[slot] = timer;
}

// 删除定时器
void removeTimer(TimeWheel* wheel, int fd) {
    // 遍历所有槽位查找要删除的定时器
    for (int i = 0; i < TIME_WHEEL_SIZE; i++) {
        Timer* prev = NULL;
        Timer* curr = wheel->slots[i];
        while (curr != NULL) {
            if (curr->fd == fd) {
                // 找到定时器，执行删除操作
                if (prev == NULL) {
                    wheel->slots[i] = curr->next;  // 删除头节点
                } else {
                    prev->next = curr->next;       // 删除中间或尾部节点
                }
                free(curr);  // 释放内存
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }
}

// 处理超时事件
void handleTimeout(TimeWheel* wheel) {
    // 获取当前槽位中的定时器链表
    Timer* timer = wheel->slots[wheel->current_slot];
    while (timer != NULL) {
        Timer* next = timer->next;  // 保存下一个定时器
        if (timer->expire_time <= time(NULL)) {
            // 如果定时器已经超时，关闭文件描述符并释放内存
            printf("踢出\n");
            LOG_INFO("Closing fd due to timeout");
            close(timer->fd);
            free(timer);
        } else {
            // 如果定时器尚未超时，将其重新插入到当前槽位
            timer->next = wheel->slots[wheel->current_slot];
            wheel->slots[wheel->current_slot] = timer;
        }
        timer = next;  // 处理下一个定时器
    }
}

// 轮转时间轮
void rotateTimeWheel(TimeWheel* wheel) {
    // 更新当前槽位到下一个槽位
    wheel->current_slot = (wheel->current_slot + 1) % TIME_WHEEL_SIZE;
}

// 将新的 net_fd 添加到 net_fd_arr 数组中
void addNetFd(int new_fd, int *net_fd_arr, int max_fds) {
    // 遍历 net_fd_arr 数组寻找空闲位置
    for (int i = 0; i < max_fds; i++) {
        if (net_fd_arr[i] == -1) {
            // 找到空闲位置，将新描述符添加到数组中
            net_fd_arr[i] = new_fd;
            break;
        }
    }
}