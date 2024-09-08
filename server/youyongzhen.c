#include "header.h"
#include "gongju.h"

/**
  ******************************************************************************
  * 功能:客户端那边上传文件过来，服务端创建文件接收客户端的文件
  * 参数:t->一个头部协议结构体（具体请观察header.h）其作用用于接收控制信息，net_fd->与客户端通信对象文件描述符
  * 返回值:正常返回0
  ******************************************************************************
**/
int putsCommand(train_t t,int net_fd){

    //获取服务端路径
    char serve_path[256]={0};
    char serve_temp[256]=BASE_PATH;

    strcpy(serve_path,serve_temp);

    printf("base路径:%s\n",serve_path);

    //获取用户路径
    char *user_path= extractParameters(t.control_msg,1);
    if(user_path==NULL){
        printf("无需拼接服务端本地路径：%s\n",serve_path);
    }else{
        strncat(serve_path,user_path, strlen(user_path));
        printf("拼接的服务端本地路径：%s\n",serve_path);
    }

    //获取文件名
    char *path_temp=extractParameters(t.control_msg,2); //path_temp获取到的参数有两种 /a/b/c.txt or c.txt

    char *path_name=NULL;
    if(path_temp[0]=='/'||path_temp[0]=='.'){
        path_name=extractFilename(path_temp);
    }else{
        path_name=path_temp;
    }
    char temp[256]={0};
    strcpy(temp,path_name);
    strncat(serve_path,temp, strlen(temp));
    strncpy(path_name,serve_path, strlen(serve_path));

    // 去掉换行符
    size_t len = strcspn(path_name, "\n");
    if (len < strlen(path_name)){
        path_name[len] = '\0'; // 将换行符替换为字符串终止符
    }

    printf("打开文件的路径名:%s\n",path_name);
    //新建文件
    int open_file_fd=open(path_name,O_WRONLY|O_CREAT|O_APPEND,0666);
    ERROR_CHECK(open_file_fd,-1,"open new file");

    //接收文件
    char buf[1024] = {0};
    ssize_t recv_num;
    while ((recv_num = recv(net_fd, buf, sizeof(buf), 0)) > 0) {
        printf("接收到 %ld 字节\n", recv_num);
        ssize_t write_file_num = write(open_file_fd, buf, recv_num);
        if (write_file_num < 0) {
            perror("write failed");
            close(open_file_fd);
            return -1;
        }
    }

    if (recv_num < 0) {
        perror("recv failed");
    } else if (recv_num == 0) {
        printf("客户端关闭了连接\n");
    }

    printf("服务器成功收到文件:%s\n",path_name);

    free(user_path);
    free(path_name);
    close(open_file_fd);
    return 0;
}