#include "header.h"

//分析协议
int analysisProtocol(train_t t, int net_fd)
{
    printf("num :%d\n",t.command);
    switch(t.command)
    {
    case LS:
        //lsCommand(t, net_fd);
        return 0;
    case CD:
        cdCommand(t, net_fd);
        return 0;
    case PWD:
        //pwdCommand(t, net_fd);
        return 0;
    case PUTS:
        putsCommand(t, net_fd);
        return 0;
    case GETS:
        //getsCommand(t, net_fd);
        //return 0;
    case REMOVE:
    case RM:
        //rmCommand(t, net_fd);
        return 0;
    default:
        printf("error:default\n");
        return 0;
    }
    return 0;
}
