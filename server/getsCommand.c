#include "header.h"

// 服务端getscommand函数实现
// 第一个参数：协议头部
// 第二个参数：网络连接net_fd
// 返回值：0为正常，其他为异常

// 判断参数个数是否正确，必须是两个
// 第一个参数：服务器虚拟路径
// 第二个参数：本地路径
// 参数异常处理
int getsCommand(train_t t, int net_fd){
    puts("in getsCommand");
    if (t.parameter_num != 2){
        t.error_flag = ABNORMAL;
        int ret = send(net_fd, &t, sizeof(t), MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }
    // 参数正常处理
    else {
        t.error_flag = NORMAL;
        int ret =send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 读取参数
    char parameter[1024] = {0};
    splitParameter(t,1,parameter);
    
    // 判断服务器参数是否错误
    if (parameter[strlen(parameter) - 1] == '/'){
        t.error_flag = ABNORMAL;
        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }
    else {
        t.error_flag = NORMAL;
        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 等待客户端那边发送检错参数
    int ret = recv(net_fd,&t,sizeof(t),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }

    // 客户端参数错误
    if (t.error_flag == ABNORMAL){
        return -1;
    }

    // 判断参数是否在根路径下
    char real_path[1024] = {0};
    if (parameter[0] == '/'){
        char tmp_path[1024] = {0};
        for (size_t i = 0; i < strlen(t.control_msg); i++){
            tmp_path[i] = t.control_msg[i];
            if (t.control_msg[i] == '/'){
                tmp_path[i] = 0;
                break;
            }
        }
        strcat(real_path,BASE_PATH);
        strcat(real_path,tmp_path);
        strcat(real_path,parameter);
    }
    // 参数不是在根路径下开始
    else {
        pathConcat(t,real_path);
        if (real_path[strlen(real_path) - 1] != '/'){
            real_path[strlen(real_path)] = '/';
        }
        strcat(real_path,parameter);
    }

    puts(real_path);

    // 在服务器中尝试打开这个文件
    int file_fd = open(real_path,O_RDONLY);
    
    // 没有这个文件会报错
    if (file_fd == -1){
        t.error_flag = ABNORMAL;
        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }
    // 有这个文件返回NORMAL
    else{
        t.error_flag = NORMAL;
        int ret = send(net_fd,&t,sizeof(t),MSG_WAITALL);
        if (ret == -1){
            printf("对端关闭\n");
        }
    }

    // 获取当前文件的hash值
    char cmd[1024] = {0};
    strcat(cmd,"sha1sum ");
    strcat(cmd,real_path);
    FILE *pipe = popen(cmd,"r");
    if (pipe == NULL){
        printf("pipe失败\n");
        return -1;
    }

    // 读取文件对应的hash值
    char shash_val[1024] = {0};
    ret = fread(shash_val,sizeof(char),sizeof(shash_val),pipe);
    ERROR_CHECK(ret,-1,"fread");
    // 关闭管道
    pclose(pipe);

    // 取出hash值
    for (size_t i = 0;i < strlen(shash_val);i++){
        if (shash_val[i] == ' '){
            shash_val[i] = 0;
            break;
        }
    }

    printf("shash_val: %s\n",shash_val);
    printf("shash_num: %ld\n",strlen(shash_val));

    // 向客户端发送当前文件的hash值
    ret = send(net_fd,shash_val,sizeof(shash_val),MSG_WAITALL);
    if (ret == -1){
        printf("对端关闭");
        return -1;
    }

    // 等待校验客户端发送是否需要重发的指令
    ret = recv(net_fd,&t,sizeof(t),MSG_WAITALL);
    if (ret == 0){
        printf("对端关闭\n");
        return -1;
    }

    // 校验对端发送的标志位，检测是否需要发送文件 
    if (t.error_flag == NORMAL){
        return -1;
    }

    // 读取文件大小
    struct stat st;
    fstat(file_fd,&st);
    int file_size = st.st_size;
    
    printf("file_size = %d\n",file_size);

    // 发送文件大小 
    ret = send(net_fd,&file_size,sizeof(file_size),MSG_NOSIGNAL);
    if (ret == -1){
        printf("对端关闭\n");
        return -1;
    }
    
    // 接收客户端发来的文件大小
    int curr_size;
    ret = recv(net_fd,&curr_size,sizeof(curr_size),MSG_NOSIGNAL);
    if (ret == -1){
        printf("对端关闭\n");
        return -1;
    }

    printf("curr_size = %d\n",curr_size);

    char aa[6] = {0};
    recv(net_fd,aa,5,MSG_WAITALL);
    puts(aa);

    // 根据文件大:发送剩余文件大小
    off_t offset = curr_size;
    size_t count = file_size - offset;
    puts("179");
    ret = sendfile(net_fd,file_fd,&offset,count);
    printf("ret = %d\n",ret);
    if (ret == -1){
        printf("发送失败\n");
        return -1;
    }
    printf("文件传输完毕\n");
    return 0;
}
