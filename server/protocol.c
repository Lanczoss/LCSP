#include "header.h"

//分析协议
int analysisProtocol(train_t *t, int net_fd, MYSQL *mysql)
{
    switch(t->command)
    {
    case LS:
        lsCommand(*t, net_fd);
        return 0;
    case CD:
        cdCommand(*t, net_fd, mysql);
        return 0;
    case PWD:
        //pwdCommand(t, net_fd);
        return 0;
    case PUTS:
        putsCommand(*t, net_fd);
        return 0;
    case GETS:
        // getsCommand(t, net_fd);
        return 0;
    case REMOVE:
        printf("函数暂时未完成！\n");
        return 0;
    case MKDIR:
        mkdirCommand(*t, net_fd, mysql);
        break;
    case RM:
        rmCommand(*t, net_fd,mysql);
        break;
    case EXIT:
        return -1;
    default:
        LOG_INFO("Wrong command.");
        break;
    }
    return 0;
}
