#include "header.h"
#include "gongju.h"

/**
  ******************************************************************************
  * 功能:客户端puts命令上传文件到服务器
  * 参数:t->一个头部协议结构体（具体请观察header.h）其作用用于接收控制信息
  *     socket->fd连接服务器对象的文件描述符
  * 返回值:正常返回0
  ******************************************************************************
**/
int putsCommand(train_t t, int socket_fd){

    //获取绝对路径
    char *client_path = getcwd(NULL, 0);
    if (client_path == NULL) {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
    strcat(client_path,"/");

    //获取用户路径
    char *user_path= extract_path_after_first_slash(extractParameters(t.control_msg,1));
    if(user_path==NULL){
        printf("客户端本地路径：%s\n",client_path);
    }else{
        strcat(client_path,user_path);
        printf("客户端本地路径：%s\n",client_path);
    }

    //参数路径名
    char *path_name= extractParameters(t.control_msg,2);
    if(path_name[0]!='/'&&path_name[0]!='.'){
        char temp[256]={0};
        strcpy(temp,path_name);
        strncat(client_path,temp, strlen(temp));
        strcpy(path_name,client_path);
    }

    // 去掉换行符
    size_t len = strcspn(path_name, "\n");
    if (len < strlen(path_name)){
        path_name[len] = '\0'; // 将换行符替换为字符串终止符
    }

    printf("打开文件的路径名:%s\n",path_name);
    //判断上传文件是否存在
    int open_file_fd=open(path_name,O_RDWR);
    if(open_file_fd==-1){
        printf("文件不存在！\n");
        return -1;
    }
    printf("成功打开%s下的文件\n",path_name);

    //发送结构体
    ssize_t send_t=send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
    ERROR_CHECK(send_t,-1,"send train_t");

    printf("发送的结构体中的字符串:%s\n",t.control_msg);

    //发送文件
    struct stat s;
    fstat(open_file_fd,&s);

    printf("发送文件的大小\n");
    ssize_t sendfile_t=sendfile(socket_fd,open_file_fd,NULL,s.st_size);

    ERROR_CHECK(sendfile_t,-1,"sendfile file");

    printf("文件发送完毕！\n");

    free(client_path);
    free(user_path);
    free(path_name);
    close(open_file_fd);
    close(socket_fd);
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
