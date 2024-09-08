#include "header.h"
#include <asm-generic/errno-base.h>
#include <complex.h>
#include "command.h"

int cdCommand(train_t t, int net_fd){
    puts("in cd command");

    printf("cd num = %d\n",t.parameter_num);

    if (t.parameter_num != 1){
        t.error_flag = ABNORMAL;
        strcpy(t.control_msg,"输入参数有误\n");
        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        if (ret == -1){
            LOG_ERROR("send:对端关闭");
        }
        return -1;
    }

    // 读取虚拟路径
    char virtual_path[1024] = {0};
    memcpy(virtual_path,t.control_msg,t.path_length);
    if (virtual_path[strlen(virtual_path) - 1] == '/'){
        virtual_path[strlen(virtual_path) - 1] = 0;
    } 

    // 读取真实路径
    char real_path[1024] = {0};
    pathConcat(t,real_path);
    if (real_path[strlen(real_path) - 1] == '/'){
        real_path[strlen(real_path) - 1] = 0;
    }

    // 读取当前层数
    int current_layers = t.current_layers;

    // 读取参数
    char parameter[1024] = {0};
    splitParameter(t,t.parameter_num,parameter);

    // 将参数最后一个空格换行符去除
    parameter[strlen(parameter) - 1] = 0;

    // 判断参数最后是否有/，没有就补
    if (parameter[strlen(parameter) - 1] != '/'){
        parameter[strlen(parameter)] = '/'; 
    }

    // 判断是否需要在根目录下寻找,若在根目录下寻找则需要重构虚拟路径与真实路径
    if(parameter[0] == '/'){
        int flag = 0;
        for (size_t i = 0;i < strlen(virtual_path);i++){
            if (virtual_path[i] == '/'){
                flag = 1;
            }
            if (flag == 1){
                virtual_path[i] = 0;
            }
        }
        strcat(real_path,BASE_PATH);
        strcat(real_path,"/");
        strcat(real_path,virtual_path);
        current_layers = 0;
    }
    
    printf("virtual_path = %s\n",virtual_path);
    printf("real_path = %s\"\n",real_path);
    printf("parameter = %s\n",parameter);
    printf("current_layers = %d\n",t.current_layers);
    printf("num = %d\n",t.parameter_num);

    // 从参数中分割单个参数 
    int pcount = 0;
    int flag_read = 0;
    while(parameter[pcount] != 0){
        char tmp_parameter[1024] = {0};
        int tpcount = 0;
        int flag = 0;
        while(1){
            if (parameter[pcount] == '/' || parameter[pcount] == 0){
                if (flag == 1){
                    pcount++;
                    flag_read++;
                    break;
                }
                if (parameter[pcount] == 0){
                    if (flag_read != 0){
                        bzero(t.control_msg,sizeof(t.control_msg));
                        memcpy(t.control_msg, virtual_path, sizeof(virtual_path));
                        t.error_flag = NORMAL;
                        t.current_layers = current_layers;
                        t.path_length = strlen(t.control_msg);
                        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                        if (ret == -1){
                            LOG_ERROR("send:对端关闭");
                        }
                        return 0;
                    }
                    else{
                        strcpy(t.control_msg,"输入有误\n");
                        t.error_flag = NORMAL;
                        strcat(virtual_path,"/");
                        memcpy(t.control_msg,virtual_path,sizeof(virtual_path));
                        t.path_length = strlen(virtual_path);
                        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                        if (ret == -1){
                            LOG_ERROR("send:对端关闭");
                        }
                        return 0;
                    }
                }
                pcount++;
                continue;
            }
            flag = 1;
            tmp_parameter[tpcount++] = parameter[pcount++];
        }

        // .保持不变
        if (strcmp(".",tmp_parameter) == 0){
            continue;
        }
        // 根据..回退上级目录
        else if (strcmp("..",tmp_parameter) == 0){
            rollbackPath(virtual_path,real_path);
            current_layers--;
            if (current_layers < 0){
                t.error_flag = ABNORMAL;
                strcpy(t.control_msg,"输入有误");
                int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                if (ret == -1){
                    LOG_ERROR("send:对端关闭");
                }
                return -1;
            }
        }
        // 若为文件名，复合要求就进行追加
        else {
            bool flag = isExistFileOrDir(real_path,tmp_parameter,1);
            if (flag == false){
                t.error_flag = ABNORMAL;
                strcpy(t.control_msg ,"输入有误");
                int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                if(ret == -1){
                    LOG_ERROR("对端关闭");
                }
                return -1;
            }
            current_layers++;
            strcat(virtual_path,"/");
            strcat(virtual_path,tmp_parameter);
            strcat(real_path,"/");
            strcat(real_path,tmp_parameter);
        }
    }
    // 将拼接好的路径发送给客户端
    bzero(t.control_msg,sizeof(t.control_msg));
    memcpy(t.control_msg, virtual_path, sizeof(virtual_path));
    t.error_flag = NORMAL;
    t.current_layers = current_layers;
    t.path_length = strlen(t.control_msg);
    int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
    if (ret == -1){
        LOG_ERROR("send:对端关闭");
    }
    return 0;
}

// 回退路径函数
// 第一个参数：虚拟路径
// 第二个参数：真实路径
// 返回值：0为正常，其他为异常
int rollbackPath(char *virtual_path, char *real_path){
    for (int i = strlen(virtual_path) - 1; i >= 0; i--){
        if (virtual_path[i] == '/'){
            virtual_path[i] = 0;
            break;
        }
        virtual_path[i] = 0;
    }

    for (int i = strlen(real_path) - 1; i >= 0; i--){
        if (real_path[i] == '/'){
            real_path[i] = 0;
            break;
        }
        real_path[i] = 0;
    }

    return 0;
}

// 判断寻找的目录或者文件是否存在
// 第一个参数：你要寻找的路径
// 第二个参数：文件名或者文件夹的名称
// 第三个参数：标志位
// flag为1表示找文件夹
// flag为0表示找文件
// 返回值：true表示目标路径下，存在文件夹或者文件
bool isExistFileOrDir(char *path, char *name, int flag){
    DIR *pdir = opendir(path);
    if (pdir == NULL){
        LOG_ERROR("opendir:打开目录流");
    }
    struct dirent *pdirent;
    while((pdirent = readdir(pdir)) != NULL){
        if (flag == 0){
            if (strcmp(pdirent->d_name,name) == 0 && pdirent->d_type == DT_REG){
                return true;
            }
        }
        else if (flag == 1){
            if (strcmp(pdirent->d_name,name) == 0 && pdirent->d_type == DT_DIR){
                return true;
            }
        }
    }
    int ret = closedir(pdir);
    if (ret == -1){
        LOG_ERROR("closedir:关闭目录流");
    }
    return false;
}

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
    extractParameters(t.control_msg,3,path_name);   //此时path_name保存的是第二个参数字符串
    printf("command函数第16行-----此时提取到的第二个参数:%s\n",path_name);
    printf("服务器结构体里面的字符串:%s\n",t.control_msg);
    if(path_name[0]!='/'&&path_name[0]!='.'){
        char temp[256]={0};
        //获取当前的服务器的路径(这里无法确定服务器路径最后是否带/,默认带/)
        //test一下
        //strcpy(temp,"/home/xiaoming/baidu_loud_storage_project/server/");
        //下面这句才是正常的
        pathConcat(t,temp);
        //将文件名拼接给path_name
        strcat(temp,path_name);
        printf("command函数第27行-----将第二个参数拼接到path的路径名:%s\n",temp);
        strcpy(path_name,temp);
    }

    // 去掉换行符
    size_t len = strcspn(path_name, "\n");
    if (len < strlen(path_name)) {
        path_name[len] = '\0'; // 将换行符替换为字符串终止符
    }

    printf("command函数第31行-----服务器保存文件路径:%s\n",path_name);

    //新建文件
    char *file_name;
    extractFilename(path_name,&file_name);
    printf("file_name :#%s# \n",file_name);
    printf("path_name:#%s#\n",path_name);

    strcat(path_name,file_name);

    int open_file_fd=open(path_name,O_RDWR|O_CREAT|O_APPEND,0666);
    ERROR_CHECK(open_file_fd,-1,"open new file");

    printf("文件已打开\n");
    
    //接收文件
    char buf[1024]={0};
    ssize_t recv_num;
    while((recv_num= recv(net_fd,buf, sizeof(buf),MSG_WAITALL))>0){
        ssize_t write_file_num=write(open_file_fd,buf,recv_num);
        ERROR_CHECK(write_file_num,-1,"write new file");
    }

    printf("服务器成功收到文件:%s\n",path_name);
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
