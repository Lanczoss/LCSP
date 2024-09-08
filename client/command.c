#include "header.h"

// 客户端cd命令请求函数
// 第一个参数：自定义协议头部
// 第二个参数：与服务端通信的sokcet_fd
// 返回值：0为正常，-1为异常表示用户退出
int cdCommand(train_t *t, int socket_fd){
    train_t tmp_t;
    int ret = recv(socket_fd,&tmp_t,sizeof(tmp_t),MSG_WAITALL);
    if (ret == 0){
        LOG_INFOR("recv:用户退出");
        return -1;
    }
    if (tmp_t.error_flag == NORMAL){
        printf("%s\n",tmp_t.control_msg);
        return 0;
    }
    memcpy(t,&tmp_t,sizeof(tmp_t));
    printf("msg:%s\n",t->control_msg);
    return 0;
}


/**
  ******************************************************************************
  * 功能:客户端puts命令上传文件到服务器
  * 参数:t->一个头部协议结构体（具体请观察header.h）其作用用于接收控制信息
  *     socket->fd连接服务器对象的文件描述符
  * 返回值:正常返回0
  ******************************************************************************
**/
int putsCommand(train_t t, int socket_fd){

    //提取文件路径名(eg: path /a/b/c.txt 提取结果为/a/b/c.txt)
    char path_name[256]={0};
    //如果第二个参数为非文件开头则需要拼接路径,否则不需要拼接
    extractParameters(t.control_msg,2,path_name);   //此时path_name保存的是第二个参数字符串
    
    // 去掉换行符
    size_t len = strcspn(path_name, "\n");
    if (len < strlen(path_name)) {
        path_name[len] = '\0'; // 将换行符替换为字符串终止符
    }
    printf("path_name:#%s#\n",path_name);
    printf("path_name_len:%ld \n",strlen(path_name));

    if(path_name[0]!='/'&&path_name[0]!='.'){
        char temp[256]={0};
        //获取当前的服务器的路径(这里无法确定服务器路径最后是否带/,默认带/)
        //test一下
        //strcpy(temp,"/home/xiaoming/");
        //下面这句才是正常的
        //pathConcat(t,temp);
        strncat(temp,t.control_msg,t.path_length);
        //将文件名拼接给path_name
        strcat(temp,path_name);
//        printf("command函数第27行-----将第二个参数拼接到path的路径名:%s\n",temp);
        strcpy(path_name,temp);
    }

//    printf("command函数第31行-----所传文件的文件路径:%s\n",path_name);

    //判断上传文件是否存在
    int open_file_fd=open(path_name,O_RDWR);
    if(open_file_fd==-1){
        printf("文件不存在！\n");
        return -1;
    }
    printf("command函数第39行-----成功打开%s下的文件\n",path_name);

    //发送结构体
    ssize_t send_t=send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
    ERROR_CHECK(send_t,-1,"send train_t");

    printf("command函数第45行-----发送的结构体中的字符串:%s\n",t.control_msg);

    //发送文件
    struct stat s;
    fstat(open_file_fd,&s);
    ssize_t sendfile_t=sendfile(socket_fd,open_file_fd,NULL,s.st_size);
    ERROR_CHECK(sendfile_t,-1,"sendfile file");

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

/**
  ******************************************************************************
  * 功能:获取文件名
  * 参数:arg->puts后面的参数，file_name->传入传出参数获取到的文件名
  * 返回值:正常返回0
  ******************************************************************************
**/
int extractFilename(const char *arg, char **file_name) {

    // 找到最后一个/
    const char *last_slash = strrchr(arg, '/');

    // 如果没有/，找到最后一个空格
    if (last_slash == NULL) {
        last_slash = strrchr(arg, ' ');
    }

    // 如果没有找到任何分隔符，则将file_name设置为NULL（或指向空字符串）
    if (last_slash == NULL) {
        return -1;
    }

    //获取文件名
    *file_name = (char *)(last_slash + 1);

    return 0;
}

int extractParameters(char *str, int num, char *arg) {
    if (str == NULL || arg == NULL || num < 1) {
        return -1;
    }

    char temp[256]={0};
    strcpy(temp,str);

    char *token;
    char *delimiters = " "; // 以空格作为第一个分隔符
    int count = 0;

    // 第一次分割，分割空格
    token = strtok(temp, delimiters);
    while (token != NULL) {
        count++;
        if (count == num) {
            // 找到目标子字符串，将其存入 arg
            strcpy(arg, token);
            return 0; // 成功
        }

        // 找到空格分割后，处理斜杠分隔
        // 找到空格分隔后的子字符串后处理斜杠
        if (count < num) {
            token = strtok(NULL, delimiters);
        }
    }

    return -1; // 未找到第 num 个子字符串
}
