#include "header.h"

void *threadMain(void *p)
{
    pool_t *pool = (pool_t*)p;
    //子线程主函数
    while(1)
    {
        //记得加锁
        int ret = pthread_mutex_lock(&pool->lock);
        THREAD_ERROR_CHECK(ret, "lock");
        //避免假唤醒
        while(pool->q.head == NULL && pool->exit_flag == 0)
        {
            ret = pthread_cond_wait(&pool->cond, &pool->lock);
            THREAD_ERROR_CHECK(ret, "cond wait");
        }
        if(pool->exit_flag == -1)
        {
            //子线程准备退出
            ret = pthread_mutex_unlock(&pool->lock);
            THREAD_ERROR_CHECK(ret, "unlock");
            pthread_exit(NULL);
        }
        //到这里说明有任务
        int net_fd;
        ret = deQueue(&pool->q, &net_fd);
        THREAD_ERROR_CHECK(ret, "deQueue");
        ret = pthread_mutex_unlock(&pool->lock);
        THREAD_ERROR_CHECK(ret, "unlock");

        //工作
        ret = doWorker(net_fd);
        if(ret == -1)
        {
            //从doWorker出来的-1是对端关闭
            printf("客户端关闭\n");
        }
        close(net_fd);
    }
    return NULL;
}

int doWorker(int net_fd)
{
    //自定义协议
    train_t t;
    bzero(&t, sizeof(t));

    //登录/注册逻辑函数
    int ret = loginRegisterSystem(&t, net_fd);
    if(ret == 0)
    {
        //对端关闭
        return -1;
    }

    //到这里开始用户成功登录
    while(1)
    {
        // 接受一次信息-》区分等下要分发给那个命令：
        ssize_t rret = recv(net_fd, &t, sizeof(t), MSG_WAITALL);
        if(rret == 0)
        {
            return -1;
        }
        //分析协议
        analysisProtocol(t, net_fd);
    }
    return 0;
}
