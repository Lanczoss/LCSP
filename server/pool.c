#include "header.h"

/* Usage:  */
//初始化线程池                  
//第一个参数是共用结构体        
int initThreads(pool_t *pool)
{
    // 从配置文件读取线程数量  
    // TODO:配置文件的测试格式eg:port=8080 err_eg:port = 8080
    // 暂时支持4个线程
    char num[16];  
    bzero(num, sizeof(num));  
    getParameter((void *)"thread_num", (void *)num);  
    

    // 将读取的字符串转换为整数  
    int thread_count = atoi(num);  
    if (thread_count <= 0) {  
        fprintf(stderr, "Invalid thread count: %s\n", num);  
        return -1; // 处理无效的线程数量  
    }  
    printf("thread_count %d \n", thread_count);
    // 线程数量  
    pool->thread_num = thread_count;  

    // 初始化队列
    bzero(&pool->q, sizeof(Queue));

    // 初始化锁
    pthread_mutex_init(&pool->lock, NULL);

    // 初始化条件变量
    pthread_cond_init(&pool->cond, NULL);

    // 初始化退出标记位
    pool->exit_flag = 0;    // 0表示正常，1表示要退出
     // 给pthread_list分配空间  
    // TODO:记得释放空间
    pool->pthread_list = (pthread_t *)malloc(thread_count * sizeof(pthread_t));  
    ERROR_CHECK(pool->pthread_list, NULL, "malloc");

    // 创建子线程
    for(int i=0; i<pool->thread_num; i++)
    {
        pthread_create(&pool->pthread_list[i], NULL, threadMain, pool);
    }
    return 0;
}

