#include "header.h"
#include "gongju.h"

/**
  ******************************************************************************
  * 功能: 客户端 puts 命令上传文件到服务器
  * 参数: t -> 一个头部协议结构体（具体请观察 header.h），用于接收控制信息
  *        socket_fd -> 连接服务器对象的文件描述符
  * 返回值: 正常返回 0，出错返回 -1
  ******************************************************************************
**/
int putsCommand(train_t t, int socket_fd) {

    printf("进入客户端没有\n");

    // 获取绝对路径
    char client_path[2048] = {0};
    getcwd(client_path, sizeof(client_path));

    printf("绝对路径%s\n",client_path);

    // 获取用户路径
    char user_path[256] = {0};
    if (extractParameters(t.control_msg, 1, user_path, sizeof(user_path)) != 0) {
        printf("无法提取用户路径\n");
        return -1;
    }

//    if (user_path[0] != '\0') {
//        strcat(client_path, "/");
//        strcat(client_path, user_path);
//    }

    printf("客户端本地路径：%s\n", client_path);

    // 获取文件名并处理路径
    char path_name[256] = {0};
    if (extractParameters(t.control_msg, 2, path_name, sizeof(path_name)) != 0) {
        printf("无法提取路径名\n");
        return -1;
    }

    if (path_name[0] != '/' && path_name[0] != '.') {
        strcat(client_path, "/");
        strcat(client_path, path_name);
    } else {
        strcpy(client_path, path_name);
    }

    // 去掉换行符
    size_t len = strcspn(client_path, "\n");
    if (len < strlen(client_path)) {
        client_path[len] = '\0'; // 将换行符替换为字符串终止符
    }

    printf("打开文件的路径名:%s\n", client_path);

    // 判断上传文件是否存在
    int open_file_fd = open(client_path, O_RDONLY);
    if (open_file_fd == -1) {
        perror("open file error");
        return -1;
    }
    printf("成功打开 %s 下的文件\n", client_path);

    // 发送结构体
    ssize_t send_t = send(socket_fd, &t, sizeof(t), MSG_NOSIGNAL);
    if (send_t == -1) {
        perror("send train_t error");
        close(open_file_fd);
        return -1;
    }

    printf("发送的结构体中的字符串: %s\n", t.control_msg);

    // 发送文件
    struct stat file_stat;
    if (fstat(open_file_fd, &file_stat) == -1) {
        perror("fstat error");
        close(open_file_fd);
        return -1;
    }

    ssize_t sendfile_t = sendfile(socket_fd, open_file_fd, NULL, file_stat.st_size);
    if (sendfile_t == -1) {
        perror("sendfile error");
        close(open_file_fd);
        return -1;
    }

    printf("文件发送完毕！\n");

    // 释放资源
    close(open_file_fd);
    return 0;
}


/**
  ******************************************************************************
  * 功能:显示客户端当前路径
  * 参数:t->保存路径的结构体
  * 返回值:正常返回0
  ******************************************************************************
**/
int pwdCommand(train_t t){

    char path_name[256]={0};
    memcpy(path_name,t.control_msg,t.path_length);
    printf("%s\n",path_name);

    return 0;
}
