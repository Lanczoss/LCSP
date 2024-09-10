#include "header.h"

void *threadMain(void *p)
{
    pool_t *pool = (pool_t*)p;
    int ret = pthread_mutex_lock(&pool->lock);
    THREAD_ERROR_CHECK(ret, "lock");
    //连接数据库
    MYSQL *mysql;
    ret = connectMysql(&mysql);
    if(ret == -1)
    {
        ret = pthread_mutex_unlock(&pool->lock);
        THREAD_ERROR_CHECK(ret, "unlock");
        printf("子线程连接数据库失败，请确保config.ini配置正确\n");
        mysql_close(mysql);
        pthread_exit(NULL);
    }
    ret = pthread_mutex_unlock(&pool->lock);
    THREAD_ERROR_CHECK(ret, "unlock");
    //子线程主函数
    while(1)
    {
        //记得加锁
        ret = pthread_mutex_lock(&pool->lock);
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
            //关闭数据库连接
            mysql_close(mysql);
            mysql_thread_end();
            pthread_exit(NULL);
        }
        //到这里说明有任务
        int net_fd;
        ret = deQueue(&pool->q, &net_fd);
        THREAD_ERROR_CHECK(ret, "deQueue");
        ret = pthread_mutex_unlock(&pool->lock);
        THREAD_ERROR_CHECK(ret, "unlock");

        //工作
        doWorker(mysql, net_fd);
        close(net_fd);
    }
    return NULL;
}

int doWorker(MYSQL *mysql, int net_fd)
{
    //自定义协议
    train_t t;
    bzero(&t, sizeof(t));

    //登录/注册逻辑函数
    int ret = loginRegisterSystem(&t, net_fd, mysql);
    if(ret == -1)
    {
        printf("对端关闭\n"); 
        return -1;
    }

    //到这里开始用户成功登录
    while(1)
    {
        bzero(&t,sizeof(t));
        // 接受一次信息-》区分等下要分发给那个命令：
        ssize_t rret = recv(net_fd, &t, sizeof(t), MSG_WAITALL);
        if(rret == 0)
        {
            printf("对端关闭\n"); 
            return -1;
        }
        //分析协议
        int ret = analysisProtocol(t, net_fd, mysql);
        if(ret == -1)
        {
            //用户输入了exit
            printf("对端关闭\n"); 
            return -1;
        }
    }
    return 0;
}
