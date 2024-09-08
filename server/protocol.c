#include "header.h"

//分析协议
int analysisProtocol(train_t t, int net_fd)
{

    switch(t.command)
    {
    case LS:
        //lsCommand(t, net_fd);
        break;
    case CD:
        //cdCommand(t, net_fd);
        break;
    case PWD:
        //pwdCommand(t, net_fd);
        break;
    case PUTS:
        printf("进函数美\n");
        putsCommand(t, net_fd);
        printf("出函数了\n");
        break;
    case GETS:
        //getsCommand(t, net_fd);
        break;
    case REMOVE:
    case RM:
        //rmCommand(t, net_fd);
        break;
    default:
        printf("error:default\n");
        break;
    }
    return 0;
}
