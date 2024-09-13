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
    int insert_index = (curr_time) % TIME_WHEEL_SIZE;
    printf("insert_index = %d\n", insert_index);

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
void handleTimeout(TimeWheel* wheel, int epoll_fd) {
    //计算需要删除的下标
    time_t curr_time = time(NULL);
    int notime_index = (curr_time + 1) % TIME_WHEEL_SIZE;
    printf("no_time index = %d\n", notime_index);
    // 获取超时定时器链表
    Timer* timer = wheel->slots[notime_index];
    while (timer != NULL) 
    {
        Timer *tmp = timer->next;
        if(timer->alive == 1)
        {
            printf("close timer fd = %d\n", timer->fd);
            close(timer->fd);
            //从epoll监听集合中删除
            struct epoll_event e;
            e.events = EPOLLIN;
            e.data.fd = timer->fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, timer->fd, &e);
        }
        free(timer);
        timer = tmp;
    }
     printf("handleTimeout handleTimeout NULL = %d\n", notime_index);
    wheel->slots[notime_index] = NULL;
}

// 将循环指针数组预备的net_fd 
// 并将该下标的链表更新位置
int checkNetFd(int fd, TimeWheel* wheel) 
{
    //计算下标
    time_t curr_time = time(NULL);
    int end_index = (curr_time) % TIME_WHEEL_SIZE;
    int start_index = (curr_time + 1) % TIME_WHEEL_SIZE;
    printf("start_index = %d, end_index = %d\n", start_index, end_index);
    while(start_index != end_index)
    {
        // 获取超时定时器链表
        Timer *timer = wheel->slots[start_index];
        printf("checkNetFd start_index = %d\n", start_index);
        while (timer != NULL)
        {
             printf("checkNetFd start_index while while = %d\n", timer->fd);
            if (fd == timer->fd && timer->alive == 1)
            {
                printf("timer fd = %d\n", timer->fd);
                timer->alive = 0;
                //更新位置
                addTimer(wheel, fd);
                return 0;
            }
            timer = timer->next;
        }
        start_index = (start_index + 1) % TIME_WHEEL_SIZE;
    }
    return -1;
}

//连接关闭从链表中移除
int removeTimer(TimeWheel* wheel, int fd, int epoll_fd)
{
    //计算下标
    // time_t curr_time = time(NULL);
    // int end_index = (curr_time) % TIME_WHEEL_SIZE;
    // int start_index = (curr_time + 1) % TIME_WHEEL_SIZE;
    for(int i = 0; i < TIME_WHEEL_SIZE; i++)
    {
        // 获取超时定时器链表
        Timer *timer = wheel->slots[i];
        while (timer != NULL)
        {
            if (timer->alive == 1 && timer->fd == fd)
            {
                //sleep(10);
                printf("超时踢出fd = %d\n", timer->fd);
                timer->alive = 0;
                //从epoll监听集合中删除
                struct epoll_event e;
                e.events = EPOLLIN;
                e.data.fd = timer->fd;
                printf("监听集合中删除 \n" );
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &e);
                close(timer->fd);
                return 0;
            }
            timer = timer->next;
        }
        //start_index = (start_index + 1) % TIME_WHEEL_SIZE;
    }
    return -1;
}