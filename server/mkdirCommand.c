#include "header.h"

int mkdirCommand(train_t t, int socket_fd,MYSQL* mysql) {

    // 获取真实路径
    char real_path[1024] = { 0 };
    pathConcat(t, real_path);


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

    return 0;
}

