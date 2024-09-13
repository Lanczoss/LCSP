#include "header.h"

int rmCommand(train_t t,int net_fd){

    int num = t.parameter_num;

    for(int i = 0;i < num ;i++){


        char buf[1024] = { 0 };
        recv(net_fd,buf,sizeof(buf),MSG_WAITALL);
        printf("%s\n",buf);

    }
    return 0;

}
