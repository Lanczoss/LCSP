#include "header.h"
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

// rmCommand 函数实现，处理客户端的 rm 命令
int rmCommand(train_t t, int net_fd,MYSQL *mysql)
{   
    //strcat(real_path, "/");

    int parma_num = t.parameter_num; 
    for(int i = 0;i < parma_num;i++){
        // 获取服务端path路径
        char real_path[1024] = { 0 };
        pathConcat(t, real_path);

        //获取文件名或者目录名
        char filename[1024] = { 0 };
        splitParameter(t, (i+1), filename);

        if(filename[0] == '/'){
            char str[] = "不能删除根目录文件夹!";
            send(net_fd,str,strlen(str),MSG_NOSIGNAL);
            return -1;
        }

        //拼接路径path+filename
        strcat(real_path, filename);
        //TODO:去除文件后面的换行符

        int ret = deleteFile(t, real_path, mysql);
        printf("ret:%d\n",ret);
        if(ret == 0){
            char str[] = "删除文件成功！";
            send(net_fd,str,strlen(str),MSG_NOSIGNAL);

        }else{
            char str[] = "删除文件失败！";
            send(net_fd,str,strlen(str),MSG_NOSIGNAL);
        }
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

