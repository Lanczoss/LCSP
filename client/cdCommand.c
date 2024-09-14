#include "header.h"

// 客户端cd命令请求函数
// 第一个参数：自定义协议头部
// 第二个参数：与服务端通信的sokcet_fd
// 返回值：0为正常，-1为异常表示用户退出
int cdCommand(train_t *t, int socket_fd){
    train_t tmp_t;
    int ret = recv(socket_fd,&tmp_t,sizeof(tmp_t),MSG_WAITALL);
    ERROR_CHECK(ret,-1,"recv");
    if (tmp_t.error_flag == ABNORMAL){
        printf("%s\n",tmp_t.control_msg);
        return 0;
    }
    puts("client cd :15");
    memcpy(t,&tmp_t,sizeof(tmp_t));
    printf("msg = %s\n",t->control_msg);
     printf("path_length= %d\n",t->path_length);
    return 0;
}

