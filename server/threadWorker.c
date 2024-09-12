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
        printf("Can't connect MySQL, please make sure config.ini correct.\n");
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
        ret = doWorker(mysql, net_fd);
        THREAD_ERROR_CHECK(ret, "One upload/download done. Check the log.");
        close(net_fd);
    }
    return NULL;
}

int doWorker(MYSQL *mysql, int net_fd)
{
    //负责上传下载
    return 0;
}
