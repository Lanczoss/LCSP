#include "header.h"

int reName(train_t t, int socket_fd){

    char buf[60] = { 0 };
    recv(socket_fd,buf,sizeof(buf),0);
    printf("%s\n",buf);

    return 0;
}
