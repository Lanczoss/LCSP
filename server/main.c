#include "header.h"

//创建目录files或者判断是否有files
int createBaseFiles(void)
{
    DIR *dir = opendir(BASE_PATH);
    if(dir == NULL)
    {
        int ret = mkdir(BASE_PATH, 0777);
        ERROR_CHECK(ret, -1, "mkdir ../files");
        LOG_INFO("Create../files success.");
    }
    LOG_INFO("Check ../files completely.");
    closedir(dir);
    return 0;
}

//判断配置文件config是否存在
int checkConfig(void)
{
    FILE *fp = fopen("config.ini", "r");
    ERROR_CHECK(fp, NULL, "Exiting because config.ini doesn't exist.");
    LOG_INFO("Check config.ini completely.");
    fclose(fp);
    return 0;
}

//自己通知自己退出
int pipefd[2];
void exit_func(int num)
{
    LOG_INFO("Got SIGINT.");
    //SIGINT信号的处理函数
    ssize_t rret = write(pipefd[1], "exit", 4);
    if(rret == -1)
    {
        LOG_PERROR("write pipe error");
        exit(-1);
    }
}
int main(void)
{
    LOG_INFO("Start cloud storage project server.");
    //创建目录files或者判断是否有files
    int ret = createBaseFiles();
    if(ret == -1)
    {
        return -1;
    }
    ret = checkConfig();
    if(ret == -1)
    {
        return -1;
    }
    //创建匿名管道
    ret = pipe(pipefd);
    ERROR_CHECK(ret, -1, "pipe");
    if(fork() != 0)
    {
        close(pipefd[0]);
        //父进程接收信号
        signal(SIGINT, exit_func);
        //等待子进程退出
        LOG_INFO("Waiting for son exit in fork.");
        wait(NULL);
        //子进程已经关闭
        exit(0);
    }
    setpgid(0, 0);
    pool_t pool;
    //线程池
    ret = initThreads(&pool);
    ERROR_CHECK(ret, -1, "init thread pools");

    //初始化socket
    int socket_fd;
    ret = initSocket(&socket_fd);
    ERROR_CHECK(ret, -1, "initSocket");

    //初始化epoll
    int epoll_fd = epoll_create(1);
    //socket_fd加入监听
    ret = addEpoll(epoll_fd, socket_fd);
    ERROR_CHECK(ret, -1, "addEpoll");

    //pipefd管道读端加入监听
    ret = addEpoll(epoll_fd, pipefd[0]);
    ERROR_CHECK(ret, -1, "addEpoll");

    LOG_INFO("Waiting for socket_fd or pipefd[0].");
    while(1)
    {
        struct epoll_event event[10];
        //开始监听
        int epoll_num = epoll_wait(epoll_fd, event, 10, -1);
        ERROR_CHECK(epoll_num, -1, "epoll_wait");

        for(int i = 0; i < epoll_num; i++)
        {
            int fd = event[i].data.fd;
            if(fd == socket_fd)
            {
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
                //关闭log文件对象
                closeLog();
                printf("\n日志已保存在.log文件\n服务已退出\n");
                //主线程退出
                pthread_exit(NULL);
            }
        }
    }
    close(socket_fd);
    close(epoll_fd);
}

