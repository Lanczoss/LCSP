#include "header.h"


int getFileName(char *src, char *file_name){
    char *str;

    // 查找最后一个斜杠的位置
    str = strrchr(src, '/');

    // 如果没有找到斜杠，返回 src 本身作为文件名
    if (str == NULL) {
        str = src;
    }else{
        // gets后是目录加文件名
        str++;
        if(str[0] == 32){
            // gets后直接是文件名
            str++;
        }
    }

    // 确保 file_name 有足够的空间
    strncpy(file_name, str, strlen(str));

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

int lsCommand(train_t t, int socket_fd){

    // 先接收错误信息,错误处理
    recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);


    if(t.error_flag == 1){
        // 错误处理
        printf("没有那个文件或目录\n");
        return -1;
    }
    off_t size;
    // 所接收文件大小
    recv(socket_fd, &size, sizeof(off_t), MSG_WAITALL);

    char buffer[1024] = { 0 };
    // 服务端ls命令结果
    recv(socket_fd, buffer, size, MSG_WAITALL);

    printf("%s",buffer);

    return 0;
}

