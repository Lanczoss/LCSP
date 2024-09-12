#include "header.h"

//分析协议
int analysisProtocol(train_t *t, int socket_fd)
{
    switch(t->command)
    {
    case LS:
        lsCommand(*t, socket_fd);
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
        getsCommand(*t, socket_fd);
        break;
    case REMOVE:
        printf("暂时未完成！\n");
        return 0;
    case MKDIR:
        mkdirCommand(t, socket_fd);
        break;
    case RM:
        rmCommand(*t, socket_fd);
        break;
    case EXIT:
        t->isLoginFailed = 1;
        printf("\n");
        break;
    case RENAME:
        reName(*t, socket_fd);
        break;
    default:
        printf("没有此命令.\n");
        break;
    }
    return 0;
}
