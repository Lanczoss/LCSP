#include "header.h"
#include <sys/epoll.h>

int addepoll(int epoll_fd,int fd){
    
    struct epoll_event event;
    event.events = EPOLLIN;

    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    return 0;
}


