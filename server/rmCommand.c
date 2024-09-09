#define _XOPEN_SOURCE 500  // Required for nftw function

#include "header.h"
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <errno.h>

// rmCommand 函数实现，处理客户端的 rm 命令
int rmCommand(train_t t, int net_fd)
{   
    // 获取服务端path路径
    char real_path[1024];
    pathConcat(t, real_path);

    printf("real_path:#%s#\n",real_path);

    //获取文件名或者目录名
    char filename[1024] = { 0 };
    splitParameter(t, 1, filename);

    //拼接路径path+filename
    strcat(real_path, filename);
    //TODO:去除文件后面的换行符
    printf("real_path:#%s#\n",real_path);

    //拼接控制命令
    char command[1024] = { 0 };
    strcat(command, "rm -rf ");

    //拼接路径
    strcat(command,  real_path);
    printf("command:#%s#\n",command);

    //调用系统调用
    int ret = system(command);
    ERROR_CHECK(ret, -1, "system");
    
    return 0;
}

