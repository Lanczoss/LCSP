#include "header.h"
#define NET_FD_NUM 30

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
int exit_flag = 0;
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
    //主线程连接数据库
    //用于登录注册和短命令
    MYSQL *mysql;
    ret = connectMysql(&mysql);
    if(ret == -1)
    {
        printf("Can't connect MySQL, please make sure config.ini correct.\n");
        mysql_close(mysql);
        pthread_exit(NULL);
    }
    //线程池用于长命令
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

    //创建net_fd循环数组（用于超时踢出）
    int *net_fd_arr = NULL;
    //暂时用malloc
    ret = createNetFdArr(&net_fd_arr, NET_FD_NUM);
    ERROR_CHECK(ret, -1, "createNetFdArr");

    LOG_INFO("Waiting for socket_fd or pipefd[0] or net_fd.");
    //自定义协议
    train_t t;
    while(1)
    {
        bzero(&t, sizeof(t));
        struct epoll_event event[10];
        //开始监听
        int epoll_num = epoll_wait(epoll_fd, event, 10, -1);
        ERROR_CHECK(epoll_num, -1, "epoll_wait");
        for(int i = 0; i < epoll_num; i++)
        {
            int fd = event[i].data.fd;
            if(fd == socket_fd)
            {
                //新连接
                //获取net_fd
                int net_fd = accept(socket_fd, NULL, NULL);
                ERROR_CHECK(net_fd, -1, "accept");
                //登录注册
                t.isLoginFailed = 1;
                //登录/注册逻辑函数
                ret = loginRegisterSystem(&t, net_fd, mysql);
                if(ret == -1)
                {
                    break;
                }
                if(t.isLoginFailed == 0 && t.isRegister == 0)
                {
                    //登录成功
                    //将net_fd加入数组并加入监听
                    addNetFd(net_fd, net_fd_arr, NET_FD_NUM);

                    //将net_fd加入监听
                    addEpoll(epoll_fd, net_fd);
                    LOG_INFO("One client login success.");
                }
                else
                {
                    close(net_fd);
                }
                
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
                //清理net_fd数组
                free(net_fd_arr);
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
            for(int j = 0; j < NET_FD_NUM; j++)
            {
                //有命令传来
                //TODO:考虑exit的情况
                if(fd == net_fd_arr[j])
                {
                    bzero(&t, sizeof(t));
                    // 接受一次信息-》区分等下要分发给那个命令：
                    ssize_t rret = recv(net_fd_arr[j], &t, sizeof(t), MSG_WAITALL);
                    if(rret == 0)
                    {
                        break;
                    }
                    //分析协议
                    int ret = analysisProtocol(&t, net_fd_arr[j], mysql);
                    if(ret == -1)
                    {
                        break;
                    }
                    else if(ret == 1)
                    {
                        ret = pthread_mutex_lock(&pool.lock);
                        THREAD_ERROR_CHECK(ret, "lock");
                        ret = enQueue(&pool.q, net_fd_arr[j]);
                        THREAD_ERROR_CHECK(ret, "enQueue");
                        ret = pthread_mutex_unlock(&pool.lock);
                        THREAD_ERROR_CHECK(ret, "unlock");
                    }
                }
            }
        }
    }
    close(socket_fd);
    close(epoll_fd);
}

