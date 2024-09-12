#include "wheel.h"
#include "header.h"

// 初始化时间轮
void initTimeWheel(TimeWheel* wheel) {
    // 将时间轮的每个槽初始化为空
    for (int i = 0; i < TIME_WHEEL_SIZE; i++) 
    {
        wheel->slots[i] = NULL;
    }
}

// 插入定时器到时间轮
int addTimer(TimeWheel* wheel, int fd) {
    //计算需要插入的下标
    time_t curr_time = time(NULL);
    int insert_index = curr_time % TIME_WHEEL_SIZE;

    // 创建一个新的结点
    Timer* new_timer = (Timer*)malloc(sizeof(Timer));
    ERROR_CHECK(new_timer, NULL, "malloc new_timer");
    new_timer->fd = fd;
    new_timer->alive = 1;
    new_timer->next = NULL;

    // 头插法
    new_timer->next = wheel->slots[insert_index];
    wheel->slots[insert_index] = new_timer;
    return 0;
}

// 处理超时事件
void handleTimeout(TimeWheel* wheel) {
    //计算需要删除的下标
    time_t curr_time = time(NULL);
    int notime_index = (curr_time % TIME_WHEEL_SIZE) - 1;

    // 获取超时定时器链表
    Timer* timer = wheel->slots[notime_index];
    while (timer != NULL) 
    {
        Timer *tmp = timer;
        timer = timer->next;
        free(tmp);
    }
}

// 将循环指针数组预备的net_fd 添加到 net_fd_arr 数组中
// 并将该下标的链表更新位置
int addNetFd(TimeWheel* wheel, int *net_fd_arr, int length) 
{
    //计算下标
    time_t curr_time = time(NULL);
    int end_index = (curr_time % TIME_WHEEL_SIZE) - 2;
    int start_index = (curr_time % TIME_WHEEL_SIZE) + 1;
    int i = 0;
    while(start_index != end_index)
    {
        // 获取超时定时器链表
        Timer *timer = wheel->slots[start_index];
        while (timer != NULL)
        {
            if (i == length)
            {
                //满
                return -1;
            }
            if (timer->alive == 1)
            {
                net_fd_arr[i] = timer->fd;
                timer->alive = 0;
                //更新位置
                addTimer(wheel, timer->fd);
                i++;
            }
            timer = timer->next;
        }
        start_index++;
    }
    return 0;
}

//连接关闭从链表中移除
int removeTimer(TimeWheel* wheel, int fd)
{
    //计算下标
    time_t curr_time = time(NULL);
    int end_index = (curr_time % TIME_WHEEL_SIZE) - 2;
    int start_index = (curr_time % TIME_WHEEL_SIZE) + 1;
    while(start_index != end_index)
    {
        // 获取超时定时器链表
        Timer *timer = wheel->slots[start_index];
        while (timer != NULL)
        {
            if (timer->alive == 1 && timer->fd == fd)
            {
                //标记为不存活
                timer->alive = 0;
                return 0;
            }
            timer = timer->next;
        }
        start_index++;
    }
    return -1;
}