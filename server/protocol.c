#include "header.h"

//分析协议
int analysisProtocol(train_t t, int net_fd)
{

    switch(t.command)
    {
    case LS:
        lsCommand(t, net_fd);
        break;
    case CD:
        cdCommand(t, net_fd);
        break;
    case PWD:
        pwdCommand(t, net_fd);
        break;
    case PUTS:
        putsCommand(t, net_fd);
        break;
    case GETS:
        getsCommand(t, net_fd);
        break;
    case REMOVE:
    case RM:
        rmCommand(t, net_fd);
        break;
    default:
        printf("error:default\n");
        break;
    }
    return 0;
}
