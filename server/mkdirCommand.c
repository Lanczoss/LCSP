#include "header.h"
#include <mysql/mysql.h>

int mkdirCommand(train_t t, int socket_fd) {

    // 获取真实路径
    char real_path[1024] = { 0 };
    pathConcat(t, real_path);

    // 获取文件夹名称
    char filename[1024] = { 0 };
    splitParameter(t, 1, filename);
    // 去掉 filename 中的换行符
    size_t filename_len = strlen(filename);
    if (filename_len > 0 && filename[filename_len - 1] == '\n') {
        filename[filename_len - 1] = '\0';
    }


    // 拼接文件夹路径并添加路径分隔符
    if (real_path[strlen(real_path) - 1] != '/') {
        strcat(real_path, "/");  // 确保路径最后有分隔符
    }
    strcat(real_path, filename);
    strcat(real_path, "/"); // 确保最终路径是一个目录


    // 删除 real_path 末尾的换行符（如果存在）
    size_t len = strlen(real_path);
    if (len > 0 && real_path[len - 1] == '\n') {
        real_path[len - 1] = '\0';
    }

    //文件路径
    printf("real_path:#%s#\n",real_path);
    printf("filename:#%s#\n",filename);

    //往数据库插入一条数据
    if( insertDir(t,real_path,filename) != 0){
        // 创建文件夹
        int ret = mkdir(real_path, 0777);
        ERROR_CHECK(ret, -1, "mkdir");

    }else{
        printf("创建目录失败!\n");
    }

    return 0;
}

