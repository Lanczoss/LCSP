#include "header.h"
#include <sys/epoll.h>

int addepoll(int epoll_fd,int fd){
    
    //创建监听集合
    struct epoll_event event;

    //注册监听行为
    event.events = EPOLLIN;
    
    //添加监听对象
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    return 0;
}


