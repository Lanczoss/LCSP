#include "header.h"

// 函数用于从文件路径中提取文件名
int get_filename(const char *path, char *buf) {
    // 查找路径中最后一个 '/' 的位置
    const char *last_slash = strrchr(path, '/');
    // 如果没有找到 '/', 则路径本身就是文件名
    if (last_slash == NULL) {
        strcpy(buf,path);
        return 0;
    }
    // 返回指向文件名的指针
    strcpy(buf,(char*)(last_slash + 1));
    return 0;
}

// 客户端接收文件函数getsCommnad
// 第一个参数：协议头部
// 第二个参数：网络连接socket_fd
// 返回值：0为正常，其他异常
int getsCommand(train_t t, int socket_fd){
    // 等待接收服务端进行参数检错
    puts("client gets getsCommand");
    int ret = recv(socket_fd,&t,sizeof(t),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }
    if (t.error_flag == ABNORMAL){
        printf("输入参数异常\n");
        return -1;
    }

    // 等待服务端检查参数是否有问题
    ret = recv(socket_fd,&t,sizeof(t),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }
    if (t.error_flag == ABNORMAL){
        printf("服务端不存在该文件\n");
        return -1;
    }

    // 处理客户端路径(相对于客户端自己主机的路径)
    char parameter[1024] = {0};
    splitParameter(t,2,parameter);
    if (parameter[strlen(parameter) - 1] == '\n'){
        parameter[strlen(parameter) - 1] = 0;
    }

    printf("para:%s\n",parameter);

    // 客户端参数检错
    struct stat st;
    ret = stat(parameter,&st);
    if (ret == -1){
        t.error_flag = ABNORMAL;
        int ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        if (ret == -1){
            printf("对端关闭\n");
            return -1;
        }
        printf("客户端输入的路径或文件不存在\n");
        return -1;
    }
    
    int file_fd; 
    if (S_ISDIR(st.st_mode)) {
        t.error_flag = NORMAL;
        int ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");

        // 拼接路径
        //// 取出文件名
        char name[1024] = {0};
        char parameter_name[1024] = {0};
        splitParameter(t,1,parameter_name);
        get_filename(parameter_name,name);

        if (parameter[strlen(parameter) - 1] == '/'){
            strcat(parameter,name);
        }

        strcat(parameter,"/");
        strcat(parameter,name);
        printf("open dir: %s\n",parameter);
        // 创建文件对象
        file_fd = open(parameter,O_RDWR | O_CREAT | O_APPEND,0666);
        ERROR_CHECK(file_fd,-1,"open");
    } else if (S_ISREG(st.st_mode)) {
        t.error_flag = NORMAL;
        int ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        file_fd = open(parameter,O_RDWR | O_CREAT | O_APPEND,0666);
        ERROR_CHECK(ret,-1,"open");
    }

    // 接收服务端文件是否存在
    ret = recv(socket_fd,&t,sizeof(t),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭");
        return -1;
    }

    if (t.error_flag == ABNORMAL){
        printf("服务端文件不存在\n");
        return -1;
    }

    // 获得客服端文件的hash值
    // 拼接指令，计算本地文件的hash值
    char cmd[1024] = {0};
    strcat(cmd,"sha1sum ");
    strcat(cmd,parameter);

    FILE *pipe = popen(cmd,"r");
    if (pipe == NULL){
        printf("pipe失败");
        return -1;
    }
    char chash_val[1024] = {0};
    ret = fread(chash_val,sizeof(char),sizeof(chash_val),pipe);
    ERROR_CHECK(ret,-1,"fread");
    // 关闭管道
    pclose(pipe);

    // 取出hash值
    for (size_t i = 0;i < strlen(chash_val);i++){
        if (chash_val[i] == ' '){
            chash_val[i] = 0;
            break;
        }
    }

    // 等待接收服务端的hash值
    char shash_val[1024] = {0};
    ret = recv(socket_fd,shash_val,sizeof(shash_val),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }
    
    printf("chash_val: %s\n", chash_val);
    printf("c num : %ld\n",strlen(chash_val));
    printf("shash_val: %s\n", shash_val);
    printf("s num : %ld\n",strlen(shash_val));

    // 本地hash值与服务端hash值进行比较标记是否需要重发
    if (strcmp(chash_val,shash_val) == 0){
        t.error_flag = NORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        if (ret == -1){
            printf("对端关闭\n");
            return -1;
        }
        printf("你有该文件无需重复下载\n");
        return -1;
    }
    else {
        t.error_flag = ABNORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        if (ret == -1){
            printf("对端关闭\n");
            return -1;
        }
    }

    // 等待接收服务端发送文件大小
    int file_size;
    ret = recv(socket_fd,&file_size,sizeof(file_size),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }
    
    printf("file_size = %d\n",file_size);

    // 计算客户端当前文件大小
    fstat(file_fd,&st);
    int curr_file_size = st.st_size;

    printf("curr_size = %d\n",curr_file_size);
    // 发送客户端的文件大小
    ret = send(socket_fd,&curr_file_size,sizeof(curr_file_size),MSG_NOSIGNAL);
    if (ret == -1){
        printf("对端关闭\n");
        return -1;
    }

    send(socket_fd,"hello",5,MSG_NOSIGNAL);

    // 计算剩余还有多少字节的内容需要接收
    int count = 0;
    while(1){
        char buf[1024] = {0};
        int tmp = recv(socket_fd,buf,sizeof(buf),MSG_DONTWAIT);
        printf("tmp = %d\n",tmp);
        printf("count = %d\n",count);
        printf("in = %d\n",file_size - curr_file_size);
        if (tmp == -1 && count == file_size - curr_file_size){
            break;
        }
        else if (tmp == 0){
            printf("对端关闭\n");
            return -1;
        }
        write(file_fd,buf,tmp);
        if (tmp != -1){
            count += tmp;
        }
    }
    printf("文件接受完毕\n");
    return 0;
}

