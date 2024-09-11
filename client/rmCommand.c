#include "header.h"

int rmCommand(train_t t,int net_fd){
    
    char buf[1024] = { 0 };
    recv(net_fd,buf,sizeof(buf),0);
    printf("%s\n",buf);
    return 0;

}
