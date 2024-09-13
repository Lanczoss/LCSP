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

// 判断id是否存在
bool isExistUid(MYSQL *mysql, char *token){
    int uid = deCodeToken(token);
    
    printf("uid = %d\n",uid);
    char select_statement[300] = {0};
    sprintf(select_statement,"select user_id from users where user_id = %d",uid);
    printf("%s\n",select_statement);
    
    int ret = mysql_query(mysql,select_statement);
    if (ret != 0){
        printf("mysql: %s\n",mysql_error(mysql));
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL){
        mysql_free_result(res);
        return false;
    }
    else {
        if (mysql_num_rows(res) == 0){
            mysql_free_result(res);
            return false;
        }
        else {
            mysql_free_result(res);
            return true;
        }
    }
}

// 处理长命令gets puts
int doWorker(MYSQL *mysql, int net_fd)
{
    puts("thread_work.c: 89\n");
    // 定义协议头部用于接受服务端的信息
    train_t t;

    // 接收客户端的token信息
    int ret = recv(net_fd,&t,sizeof(t),MSG_WAITALL);
    ERROR_CHECK(ret,-1,"recv");

    printf("token: %s\n",t.token);

    // 拿到解析出的uid进行查表，观察是否有这个uid
    bool flag = isExistUid(mysql,t.token);
    
    // 根据flag进行判断对方的用户身份
    if (flag == false){
        t.error_flag = ABNORMAL;
        ret = send(net_fd,&t,sizeof(t),MSG_WAITALL);
        ERROR_CHECK(ret,-1,"send");
        LOG_INFO("用户身份错误");
        return -1;
    }
    else {
        puts("113");
        t.error_flag = NORMAL;
        printf("flag = %d\n",t.error_flag);
        ret = send(net_fd,&t,sizeof(t),MSG_WAITALL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 处理长命令
    switch(t.command){
        case GETS:
            ret = getsCommand(t,net_fd,mysql);
            ERROR_CHECK(ret,-1,"getsCommand");
        case PUTS:
            ret = putsCommand(t,net_fd,mysql);
            ERROR_CHECK(ret,-1,"putsCommand");
            break;
        default:
            LOG_INFO("长命令解析错误");
            return -1;
    }
    return 0;
}
