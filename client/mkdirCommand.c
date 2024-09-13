#include "header.h"

//mkdir的命令
int mkdirCommand(train_t *t, int socket_fd){

    //send(socket_fd,&t,sizeof(train_t),MSG_NOSIGNAL);
    printf("正在创建文件!\n");
    int num = t->parameter_num;
    for(int i = 0;i < num ;i++){

        char message[1024] = { 0 };
        recv(socket_fd,message,sizeof(message),0);
        printf("%s\n",message);
    }
    return 0;
}
