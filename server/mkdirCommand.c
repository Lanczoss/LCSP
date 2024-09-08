#include "header.h"

int mkdirCommand(train_t t, int socket_fd){

    //获取真实路径
    char real_path[1024] = { 0 };
    pathConcat(t,real_path);

    //获取文件夹名称
    char filename[1024] = { 0 };
    splitParameter(t,1,filename);

    //获取绝对路径
    strcat(real_path,filename);
    //删除换行符
    size_t len = strlen(real_path);
    if (len > 0 && real_path[len - 1] == '\n') {
        real_path[len - 1] = '\0';
    }
    
    //创建文件夹
    int ret = mkdir(real_path,0777);
    ERROR_CHECK(ret, -1, "mkdir");


    return 0;
}


