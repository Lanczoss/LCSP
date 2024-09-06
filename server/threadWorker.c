#include "header.h"

void *threadMain(void *p)
{
    pool_t *pool = (pool_t*)p;
    //子线程主函数
    while(1)
    {
        //拿到net_fd
        int net_fd;
        //从队列中取net_fd
        //记得加锁
        deQueue(&pool->q, &net_fd);
        //工作
        doWorker(net_fd);

        close(net_fd);
    }
}

int doWorker(int net_fd)
{
    while(1)
    {
        //自定义协议
        train_t t;

        // TODO:接受一次信息-》区分等下要分发给那个命令：

        //分析协议
        analysisProtocol(&t, net_fd);

        //


    }
}
