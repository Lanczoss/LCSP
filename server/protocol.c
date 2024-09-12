#include "header.h"

//分析协议
int analysisProtocol(train_t *t, int net_fd, MYSQL *mysql)
{
    //短命令返回0
    //长命令返回1
    //exit返回-1
    switch(t->command)
    {
    case LS:
        lsCommand(*t, net_fd, mysql);
        break;
    case CD:
        cdCommand(*t, net_fd, mysql);
        break;
    case PWD:
        //pwdCommand(t, net_fd);
        break;
    case PUTS:
        //putsCommand(*t, net_fd);
        return 1;
    case GETS:
        // getsCommand(t, net_fd);
        return 1;
    case REMOVE:
        printf("函数暂时未完成！\n");
        break;
    case MKDIR:
        mkdirCommand(*t, net_fd, mysql);
        break;
    case RM:
        rmCommand(*t, net_fd,mysql);
        break;
    case EXIT:
        return -1;
    case RENAME:
        reName(*t,net_fd,mysql);
        break;
    default:
        LOG_INFO("Wrong command.");
        break;
    }
    return 0;
}
