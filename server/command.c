#include "header.h"

/**
  ******************************************************************************
  * 功能:客户端那边上传文件过来，服务端创建文件接收客户端的文件
  * 参数:t->一个头部协议结构体（具体请观察header.h）其作用用于接收控制信息，net_fd->与客户端通信对象文件描述符
  * 返回值:正常返回0
  ******************************************************************************
**/
int putsCommand(train_t t,int net_fd){

    //提取文件路径名(eg path /a/b/c.txt 提取结果为/a/b/c.txt)
    char path_name[256]={0};
    //如果第二个参数为非文件开头则需要拼接路径,否则不需要拼接
    extractParameters(t.control_msg,2,path_name);   //此时path_name保存的是第二个参数字符串
//    printf("command函数第16行-----此时提取到的第二个参数:%s\n",path_name);
//    printf("服务器结构体里面的字符串:%s\n",t.control_msg);
    if(path_name[0]!='/'&&path_name[0]!='.'){
        char temp[256]={0};
        //获取当前的服务器的路径(这里无法确定服务器路径最后是否带/,默认带/)
        //test一下
        //strcpy(temp,"/home/xiaoming/baidu_loud_storage_project/server/");
        //下面这句才是正常的
        pathConcat(t,temp);
        //将文件名拼接给path_name
        strcat(temp,path_name);
//        printf("command函数第27行-----将第二个参数拼接到path的路径名:%s\n",temp);
        strcpy(path_name,temp);
    }

//    printf("command函数第31行-----服务器保存文件路径:%s\n",path_name);

    //新建文件
    int open_file_fd=open(path_name,O_WRONLY|O_CREAT|O_APPEND,0666);
    ERROR_CHECK(open_file_fd,-1,"open new file");

    //接收文件
    char buf[1024]={0};
    ssize_t recv_num;
    while((recv_num= recv(net_fd,buf, sizeof(buf),MSG_WAITALL))>0){
        ssize_t write_file_num=write(open_file_fd,buf,recv_num);
        ERROR_CHECK(write_file_num,-1,"write new file");
    }

//    printf("服务器成功收到文件:%s\n",path_name);
    close(open_file_fd);
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

/**
  ******************************************************************************
  * 功能:以空格为分割，提取指定参数(eg: "path cd ../a/b/c.txt" 提取第二个参数cd)
  * 参数:str->字符串，num->第几个参数，arg->所提取的参数
  * 返回值:
  ******************************************************************************
**/
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
