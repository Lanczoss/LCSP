#include "header.h"


int getFilePath(char *src, char *file_name){
    char *token;

    // 获取第一个子字符串
    token = strtok(src, " ");

    // 将第二个字符串拼接形成文件路径
    token = strtok(NULL, " ");
    strncat(file_name, token, strlen(token));

    return 0;
}

int removeLineBreak(char *real_path){
    // 找到换行符的位置
    size_t len = strcspn(real_path, "\n");

    // 如果找到了换行符，len 小于路径名的长度
    if (len < strlen(real_path)) {
        real_path[len] = '\0'; // 将换行符替换为字符串终止符
    }

    return 0;
}

int getsCommand(train_t t, int net_fd){
    char real_path[1024] = { 0 };

    // 获得文件的绝对路径
    pathConcat(t, real_path);
    

    // 拼接文件的真实路径
    char src[1024] = { 0 };
    strcpy(src,t.control_msg);
    getFilePath(src, real_path);

    // 去掉路径中的换行符
    removeLineBreak(real_path);
     
    // 打开文件

    int file_fd = open(real_path, O_RDWR);

    if(file_fd == -1){
        // 文件不存在，直接返回错误信息
        // TODO 修改错误标志
        t.error_flag = 1;
        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);
        return -1;
    }

    send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);

    // 发送文件
    // 获取文件大小
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(file_fd, &st);

    // 先发文件大小给客户端
    send(net_fd, &st.st_size, sizeof(off_t), MSG_NOSIGNAL);

    // 使用sendfile:零拷贝
    sendfile(net_fd, file_fd, NULL, st.st_size);

    close(file_fd);

    return 0;
}


int lsCommand(train_t t, int net_fd){

    char real_path[1024] = { 0 };
    pathConcat(t, real_path);


    // 将路径拼接在命令后构成在服务端运行的完整命令
    char cmd[1024] = "ls ";
    if(t.parameter_num == 1){
        char src[1024] = { 0 };
        strcpy(src, t.control_msg);
        getFilePath(src, real_path);
        strncat(cmd, real_path, strlen(real_path));

    }
    strncat(cmd, real_path, strlen(real_path));


    removeLineBreak(real_path);
    
    DIR *ret_open = opendir(real_path);
    
    
    if(ret_open == NULL){
        t.error_flag = 1;
        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);
        closedir(ret_open);
        return -1;

    }
    closedir(ret_open);
    send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);

    FILE *fp;
    char buffer[1024] = { 0 };
    char send_msg[1024] = { 0 };
    // 打开popen管道,执行命令
    fp = popen(cmd, "r");


    // 将命令的输出读取到send_msg里
    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        strncat(send_msg, buffer, strlen(buffer));
    }

    pclose(fp);

    // 发送传输文件大小
    off_t size = strlen(send_msg) + 1;

    send(net_fd, &size, sizeof(off_t), MSG_NOSIGNAL);

    // 发送ls执行后的输出结果
    send(net_fd, send_msg, size, MSG_NOSIGNAL);

    return 0;
}

