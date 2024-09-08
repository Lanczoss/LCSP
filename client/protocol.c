#include "header.h"

//分析协议
int analysisProtocol(train_t *t, int socket_fd)
{
    switch(t->command)
    {
    case LS:
        //lsCommand(t, socket_fd);
        break;
    case CD:
        cdCommand(t, socket_fd);
        break;
    case PWD:
        pwdCommand(*t);
        break;
    case PUTS:
        putsCommand(*t, socket_fd);
        break;
    case GETS:
        //getsCommand(t, socket_fd);
        break;
    case REMOVE:
    case RM:
        //rmCommand(t, socket_fd);
        break;
    default:
        printf("error:default\n");
        break;
    }
    return 0;
}
