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

