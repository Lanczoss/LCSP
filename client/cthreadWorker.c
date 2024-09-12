#include "header.h"

// 子线程主函数
void *thread_main(void *args){
    // 转换参数
    thread_args *thread_t = (thread_args *)args;

    // 服务端需要将自己token发送给服务端
    int ret = send(thread_t->socket_fd,thread_t->t,sizeof(train_t),MSG_NOSIGNAL);
    ERROR_CHECK(ret,-1,"send");

    // 等待服务端验证用户身份
    ret = recv(thread_t->socket_fd,thread_t->t,sizeof(thread_t),MSG_WAITALL);
    ERROR_CHECK(ret,-1,"recv");

    // 检测身份是否有异常
    if (thread_t->t->error_flag == ABNORMAL){
        printf("身份有误\n");
        close(thread_t->socket_fd);
        return (void *)-1;
    }

    // 长命令进行gets和puts的区分
    switch(thread_t->t->command){
        case GETS:
            ret = getsCommand(*thread_t->t, thread_t->socket_fd);
            ERROR_CHECK(ret,-1,"getsCommand");
            break;
        case PUTS:
            putsCommand(*thread_t->t, thread_t->socket_fd);
            ERROR_CHECK(ret,-1,"putsCommand");
            break;
        default:
            printf("输入命令错误");
    }
    return NULL;
}