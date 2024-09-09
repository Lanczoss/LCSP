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


int getsCommand(train_t t, int socket_fd){


    // 接收是否返回信息，判断文件是否可以下载
    recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);


    // TODO 对返回信息判断
    if(t.error_flag == 1){
        // 服务端文件不存在
        printf("文件不存在,下载失败！\n");
        return -1;
    }
    // 到这里可下载
    // 获取文件名
    char file_name[1024] = { 0 };
    getFileName(t.control_msg, file_name);
    removeLineBreak(file_name);


    // 创建文件
    int file_fd = open(file_name, O_RDWR | O_CREAT, 0666);

    // 接收文件大小
    off_t file_size = 0;
    recv(socket_fd, &file_size, sizeof(off_t), MSG_WAITALL);

    // mmap之前修改文件大小
    ftruncate(file_fd, file_size);

    // 接收数据
    void *p = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);

    recv(socket_fd, p, file_size, MSG_WAITALL);

    munmap(p, file_size);

    close(file_fd);
    
    printf("文件下载成功\n");

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

