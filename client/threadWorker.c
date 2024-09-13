#include "header.h"

// 子线程主函数
void *thread_main(void *args){
    // 转换参数
    train_t t = *(train_t *)args;

    // 创建socket对象
    int socket_fd = socket(AF_INET,SOCK_STREAM,0);
    if (socket_fd == -1){
        LOG_PERROR("socket");
        return (void *)-1;
    }

    // 取出参数
    char port[100] = {0};
    char ip[100] = {0};
    getParameter("ip",ip);
    getParameter("port",port);

    // 建立连接
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(port));
    sockaddr.sin_addr.s_addr = inet_addr(ip);

    puts("client_main 26");
    int ret = connect(socket_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (ret == -1){
        LOG_PERROR("connect");
        return (void *)-1;
    }
    puts("client_main 33");

    // 客户端向服务端发送标记位
    t.isDownloadOrUpload = true;
    ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
    if (ret == -1){
        LOG_PERROR("send");
        return (void *)-1;
    }

    // 服务端需要将自己token发送给服务端
    ret = send(socket_fd,&t,sizeof(train_t),MSG_NOSIGNAL);
    if (ret == 0){
        LOG_PERROR("send");
        return (void *)-1;
    }

    // 等待服务端验证用户身份
    ret = recv(socket_fd,&t,sizeof(train_t),MSG_WAITALL);
    if (ret == -1){
        LOG_PERROR("recv");
        return (void *)-1;
    }
    
    // 检测身份是否有异常
    if (t.error_flag == ABNORMAL){
        printf("身份有误\n");
        close(socket_fd);
        return (void *)-1;
    }

    // 长命令进行gets和puts的区分
    switch(t.command){
        case GETS:
            puts("62: getsCommand");
            ret = getsCommand(t, socket_fd);
            if (ret == -1){
                LOG_INFO("getsCommand");
                return (void *)-1;
            }
            break;
        case PUTS:
            puts("70: putsCommand");
            ret = putsCommand(t, socket_fd);
            if (ret == -1){
                LOG_INFO("putsCommand");
                return (void *)-1;
            }
            break;
        default:
            printf("输入命令错误");
    }
    close(socket_fd);
    puts("end: 89");
    return 0;
}