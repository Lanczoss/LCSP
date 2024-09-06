#include "header.h"

int pipefd[2];
int main(void)
{
    if(fork() != 0)
    {
        //父进程接收信号

    }
    setpgid(0, 0);
    pool_t pool;
    //线程池
    int ret = initThreads(&pool);

    //初始化socket
    int socket_fd;
    ret = initSocket(&socket_fd);

    //初始化epoll
    int epoll_fd = epoll_create(1);
    ret = addEpoll(epoll_fd, socket_fd);
    ret = addEpoll(epoll_fd, pipefd[0]);

    while(1)
    {
        struct epoll_event event[10];
        //开始监听
        int epoll_num = epoll_wait(epoll_fd, event, 10, -1);

        for(int i = 0; i < epoll_num; i++)
        {
            int fd = event[i].data.fd;
            if(fd == socket_fd)
            {
                //监听socket_fd
                int net_fd = accept(socket_fd, NULL, NULL);
                //放入队列
                //记得加锁
                enQueue(&pool.q, net_fd);
                //唤醒

            }
            if(fd == pipefd[0])
            {

            }
        }
    }
    close(socket_fd);
    close(epoll_fd);
}
