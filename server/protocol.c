#include "header.h"

//分析协议
int analysisProtocol(train_t t, int net_fd)
{
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
        printf("进函数美\n");
        putsCommand(t, net_fd);
        printf("出函数了\n");
        return 0;
    case GETS:
        //getsCommand(t, net_fd);
        //return 0;
    case REMOVE:
        printf("函数暂时未完成！\n");
        return 0;
    case MKDIR:
        mkdirCommand(t, net_fd);
    case RM:
        //rmCommand(t, net_fd);
        return 0;
    default:
        printf("error:default\n");
        return 0;
    }
    return 0;
}
