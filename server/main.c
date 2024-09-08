#include "header.h"

//创建目录files或者判断是否有files
int createBaseFiles(void)
{
    printf("检测是否有../files文件夹\n");
    DIR *dir = opendir(BASE_PATH);
    if(dir == NULL)
    {
        //没有此文件夹
        printf("不存在，正在创建\n");
        int ret = mkdir(BASE_PATH, 0777);
        if(ret == -1)
        {
            return -1;
        }
    }
    printf("检测完成，正在启动服务\n");
    closedir(dir);
    return 0;
}

//判断配置文件config是否存在
int checkConfig(void)
{
    FILE *fp = fopen("config.ini", "r");
    if(fp == NULL)
    {
        printf("config.ini不存在，服务正在退出\n");
        return -1;
    }
    fclose(fp);
    return 0;
}

//自己通知自己退出
int pipefd[2];
void exit_func(int num)
{
    //SIGINT信号的处理函数
    ssize_t rret = write(pipefd[1], "exit", 4);
    if(rret == -1)
    {
        printf("write pipe error.\n");
        exit(-1);
    }
}
int main(void)
{
    //创建目录files或者判断是否有files
    int ret = createBaseFiles();
    if(ret == -1)
    {
        printf("创建基本文件夹files失败");
        exit(-1);
    }
    ret = checkConfig();
    if(ret == -1)
    {
        exit(1);
    }
    //创建匿名管道，注册2号信号
    ret = pipe(pipefd);
    ERROR_CHECK(ret, -1, "pipe");
    if(fork() != 0)
    {
        close(pipefd[0]);
        //父进程接收信号
        signal(SIGINT, exit_func);
        //等待子进程退出
        wait(NULL);
        //子进程已经关闭
        exit(0);
    }
    setpgid(0, 0);
    pool_t pool;
    //线程池
    ret = initThreads(&pool);
    ERROR_CHECK(ret, -1, "initThreads");

    //初始化socket
    int socket_fd;
    ret = initSocket(&socket_fd);
    ERROR_CHECK(ret, -1, "initSocket");

    //初始化epoll
    int epoll_fd = epoll_create(1);
    //socket_fd加入监听
    ret = addEpoll(epoll_fd, socket_fd);
    ERROR_CHECK(ret, -1, "addEpoll socket_fd");
    //pipefd管道读端加入监听
    ret = addEpoll(epoll_fd, pipefd[0]);
    ERROR_CHECK(ret, -1, "addEpoll epoll_fd");

    while(1)
    {
        struct epoll_event event[10];
        //开始监听
        int epoll_num = epoll_wait(epoll_fd, event, 10, -1);
        ERROR_CHECK(epoll_num, -1, "epoll wait");

        for(int i = 0; i < epoll_num; i++)
        {
            int fd = event[i].data.fd;
            if(fd == socket_fd)
            {
                //如果是监听到socket对象读端
                //获取net_fd
                int net_fd = accept(socket_fd, NULL, NULL);
                ERROR_CHECK(net_fd, -1, "accept");
                //放入队列
                //记得加锁
                ret = pthread_mutex_lock(&pool.lock);
                ERROR_CHECK(ret, -1, "get lock");
                ret = enQueue(&pool.q, net_fd);
                ERROR_CHECK(ret, -1, "enQueue");
                //广播唤醒
                ret = pthread_cond_broadcast(&pool.cond);
                ERROR_CHECK(ret, -1, "cond broadcast");
                //解锁
                ret = pthread_mutex_unlock(&pool.lock);
                ERROR_CHECK(ret, -1, "unlock");
            }
            if(fd == pipefd[0])
            {
                //监听到管道读端
                //准备退出
                char buf[10] = {0};
                ssize_t rret = read(pipefd[0], buf, sizeof(buf));
                ERROR_CHECK(rret, -1, "read pipe");
                //加锁
                ret = pthread_mutex_lock(&pool.lock);
                ERROR_CHECK(ret, -1, "get lock");
                //修改退出标记位
                pool.exit_flag = -1;
                //唤醒解锁
                ret = pthread_cond_broadcast(&pool.cond);
                ERROR_CHECK(ret, -1, "cond broadcast");
                ret = pthread_mutex_unlock(&pool.lock);
                ERROR_CHECK(ret, -1, "unlock");

                //等待子线程退出
                for(int j = 0; j < 0; j++)
                {
                    ret = pthread_join(pool.pthread_list[j], NULL);
                    ERROR_CHECK(ret, -1, "pthread join");
                }

                //清理队列
                while(pool.q.head != NULL)
                {
                    int net_fd;
                    deQueue(&pool.q, &net_fd);
                    close(net_fd);
                }
                //清理存储线程id的链表
                free(pool.pthread_list);
                //清理主线程资源
                close(socket_fd);
                close(epoll_fd);
                //主线程退出
                pthread_exit(NULL);
            }
        }
    }
    close(socket_fd);
    close(epoll_fd);
}

