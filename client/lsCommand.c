#include "header.h"

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
    // 参数大于1，等待服务端发送的错误信息
    if(t.parameter_num > 1){
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);
        if(t.error_flag == 1){
            printf("参数个数不合法，请输入合法参数\n");
        }
        return 0;
    }

    // 参数个数为0，ls查表一定成功，接收结果
    if(t.parameter_num == 0){
        // 为了兼容下面
        // 服务端多send了一次，多recv一次
        recv(socket_fd, &t, sizeof(t), MSG_WAITALL);

        // 先接收是否有错误信息
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

        // 错误标志如果为3，说明查表成功
        // 但未查到数据
        // 即当前目录下无内容
        // 打印换行
        if(t.error_flag == 3){
            printf("当前目录下无内容\n");
            return 0;
        }
        // 查到数据进行打印
        int file_size;
        char buffer[1024] = { 0 };

        // 先接文件大小，再接内容
        recv(socket_fd, &file_size, sizeof(int), MSG_WAITALL);

        recv(socket_fd, buffer, file_size, MSG_WAITALL);
        
        // 如果字符数组无内容
        // TODO
        printf("%s\n",buffer);

        return 0;
    }

    // 到这儿说明ls后为1个参
    char path[256] = { 0 };
    char para[256] = { 0 };

    // 获得客户端目前路径
    strncpy(path, t.control_msg, t.path_length);
    // 获得参数
    splitParameter(t, 1, para);

    // 如果参数不以斜杠开头,添加一个斜杠
    if(para[0] != '/'){
        size_t len = strlen(para);
        memmove(para + 1, para, len + 1);
        para[0] = '/';
    }
    
    // 去掉para后的换行符
    removeLineBreak(para);

    
    // 客户端目前路径和参数一样
    // 相当于ls无参
    if(strcmp(path, para) == 0){
        // 为了兼容下面，服务端多发了一次
        // 多recv一次
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

        // 先接收是否有错误信息
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

        // 错误标志如果为3，说明查表成功
        // 但未查到数据
        // 即当前目录下无内容
        // 打印换行
        if(t.error_flag == 3){
            printf("当前目录下无内容\n");
            return 0;
        }
        // 查到数据进行打印
        int file_size;
        char buffer[1024] = { 0 };

        // 先接文件大小，再接内容
        recv(socket_fd, &file_size, sizeof(int), MSG_WAITALL);

        recv(socket_fd, buffer, file_size, MSG_WAITALL);

        printf("%s\n",buffer);

        return 0;

    }
    
    // 进入判读参数合法
    if(strlen(para) == 1 && para[0] == '/'){
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);
        if(t.error_flag == 4){
            printf("非法参数，参数个数不能大于1！\n");
        }
        return 0;
    }
    

    // 检查参数是否是以当前路径开头
    if(strncmp(para, path, t.path_length) == 0){
        // 这里需检查错误标志是否为2
        // 表中没有以当前参数为路径的记录
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

        if(t.error_flag == 2){
            // 说明当前目录下无该目录或文件
            printf("当前目录下无该目录或文件\n");
            return 0;
        }

        // 先接收是否有错误信息
        recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

        // 错误标志如果为3，说明查表成功
        // 但未查到数据
        // 即当前目录下无内容
        // 打印换行
        if(t.error_flag == 3){
            printf("当前目录下无内容\n");
            return 0;
        }
        // 查到数据进行打印
        int file_size;
        char buffer[1024] = { 0 };

        // 先接文件大小，再接内容
        recv(socket_fd, &file_size, sizeof(int), MSG_WAITALL);

        recv(socket_fd, buffer, file_size, MSG_WAITALL);

        printf("%s\n",buffer);

        return 0;

    }



    // 到这里就是参数不以路径开头，且不相同


    // 这里需检查错误标志是否为2
    // 表中没有以当前参数为路径的记录
    recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

    if(t.error_flag == 2){
        // 说明当前目录下无该目录或文件
        printf("当前目录下无该目录或文件\n");
        return 0;
    }


    // 先接收是否有错误信息
    recv(socket_fd, &t, sizeof(train_t), MSG_WAITALL);

    // 错误标志如果为3，说明查表成功
    // 但未查到数据
    // 即当前目录下无内容
    // 打印换行
    if(t.error_flag == 3){
        printf("当前目录下无内容\n");
        return 0;
    }
    // 查到数据进行打印
    int file_size;
    char buffer[1024] = { 0 };

    // 先接文件大小，再接内容
    recv(socket_fd, &file_size, sizeof(int), MSG_WAITALL);

    recv(socket_fd, buffer, file_size, MSG_WAITALL);

    printf("%s\n",buffer);


    return 0;
}

