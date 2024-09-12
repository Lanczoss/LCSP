#include "header.h"

int mkdirCommand(train_t t, int socket_fd,MYSQL* mysql) {

    // 获取真实路径
    char real_path[1024] = { 0 };
    pathConcat(t, real_path);

    // 拼接文件夹路径并添加路径分隔符
    /*
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
    // 创建文件夹
    int ret = mkdir(real_path, 0777);
    ERROR_CHECK(ret, -1, "mkdir");*/

    int parma_num = t.parameter_num;

    for(int i = 0;i < parma_num;i++){

        // 获取文件夹名称
        char filename[1024] = { 0 };
        splitParameter(t, (i+1), filename);
        LOG_PERROR("splitParameter");
        // 去掉 filename 中的换行符
        size_t filename_len = strlen(filename);
        //文件名不能以"/"开头
        if(filename[0] == '/'){
            char message[] = "权限不足，文件夹名称不能以'/'开头";
            send(socket_fd,message,strlen(message),MSG_NOSIGNAL);
            return 0;
        }
        
        if (filename_len > 0 && filename[filename_len - 1] == '\n') {
            filename[filename_len - 1] = '\0';
        }
        if(insertDir(t,real_path,filename,mysql) == 0){
            LOG_INFO("创建文件夹成功！\n");
            char str[] = "创建文件夹成功!";
            send(socket_fd,str,strlen(str),MSG_NOSIGNAL);
        }else{
            char str[] = "创建文件夹失败！\n";
            send(socket_fd,str,strlen(str),MSG_NOSIGNAL);
        }
    }
    
    //测试Token
    char buf[4096] = { 0 };
    int ret = enCodeToken(t,buf);
    printf("测试加密---------------------------\n");
    printf("jwt_token:%s\n",buf);

    printf("解密测试---------------------------\n");
    ret = deCodeToken(buf);
    printf("uid:%d\n",ret);

    return 0;
}

