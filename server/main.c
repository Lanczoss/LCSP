#include <cpp61th.h>

#include "header.h"

int main(int argc, char *argv[])
{
    int socket_fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi("8080"));
    sockaddr.sin_family = inet_addr("192.168.101.128");

    bind(socket_fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr));

    int net_fd = accept(socket_fd,NULL,NULL);

    train_t t;
    while(1){
       recv(net_fd,&t,sizeof(t),MSG_WAITALL);
       cdHandleCommand(t,net_fd);
    }
    return 0;
}

