#include "header.h"

//分析协议
int analysisProtocol(train_t t, int net_fd)
{
<<<<<<< HEAD

=======
>>>>>>> 29650e48a9c883b0f3ef1fa84e3930cf0ab86ba9
    switch(t.command)
    {
    case LS:
        //lsCommand(t, net_fd);
        return 0;
    case CD:
<<<<<<< HEAD
        //cdCommand(t, net_fd);
        break;
=======
        cdCommand(t, net_fd);
        return 0;
>>>>>>> 29650e48a9c883b0f3ef1fa84e3930cf0ab86ba9
    case PWD:
        //pwdCommand(t, net_fd);
        return 0;
    case PUTS:
        printf("进函数美\n");
        putsCommand(t, net_fd);
<<<<<<< HEAD
        printf("出函数了\n");
        break;
=======
        return 0;
>>>>>>> 29650e48a9c883b0f3ef1fa84e3930cf0ab86ba9
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
