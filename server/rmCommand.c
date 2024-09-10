#include "header.h"
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

// rmCommand 函数实现，处理客户端的 rm 命令
int rmCommand(train_t t, int net_fd,MYSQL *mysql)
{   

    // 获取服务端path路径
    char real_path[1024];
    pathConcat(t, real_path);
    //strcat(real_path, "/");

    //获取文件名或者目录名
    char filename[1024] = { 0 };
    splitParameter(t, 1, filename);

    //拼接路径path+filename
    strcat(real_path, filename);
    //TODO:去除文件后面的换行符

    int ret = deleteFile(real_path,mysql);
    printf("ret:%d\n",ret);
    if(ret == 0){
        printf("删除文件成功\n");
    }else{
        printf("删除文件失败!\n");
    }


    //拼接控制命令
    /*
    char command[1024] = { 0 };
    strcat(command, "rm -rf ");

    //拼接路径
    strcat(command,  real_path);

    //调用系统调用
    int ret = system(command);
    ERROR_CHECK(ret, -1, "system");*/
    
    return 0;
}

