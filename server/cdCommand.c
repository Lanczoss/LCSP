#include "header.h"

// 回退路径函数
// 第一个参数：虚拟路径
// 第二个参数：真实路径
// 返回值：0为正常，其他为异常
int rollbackPath(char *virtual_path){
    for (int i = strlen(virtual_path) - 1; i >= 0; i--){
        if (virtual_path[i] == '/'){
            virtual_path[i] = 0;
            break;
        }
        virtual_path[i] = 0;
    }
    return 0;
}

// 判断寻找的目录或者文件是否存在
// 第一个参数：mysql数据库指针
// 第二个参数：协议头部
// 第三个参数：当前查询的虚拟路
// 返回值：true表示目标路径下，存在文件夹或者文件
bool isExistDir(MYSQL *mysql, train_t t, char *virtual_path, char *parameter){
    char select_statement[1024] = {0};
    char error_statement[1024] = {0};
    sprintf(select_statement,"select * from files where uid = %d and file_type = %d and file_path = '%s/%s'",t.uid,1,virtual_path,parameter);
    printf("st: %s\n",select_statement);
    int ret = mysql_query(mysql,select_statement);
    if (ret != 0){
        strcat(error_statement,"mysql_query");
        strcat(error_statement,":");
        strcat(error_statement,mysql_error(mysql));
        printf("%s\n",error_statement);
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL){
        return false;
    } 
    else {
        int row_num = mysql_num_rows(res);
        if (row_num == 0){
            return false;
        }
        else {
            return true;
        }
    }
}

// cd命令实现
// 第一个参数：协议头部
// 第二个参数：网络连接net_fd
// 第三个参数：mysql数据库连接
// 返回值：0为正常，-1为异常
int cdCommand(train_t t, int net_fd, MYSQL *mysql){
    printf("t.msg = %s\n",t.control_msg);
    // 检错返回值
    int ret;
    // cd参数必须要保证为1个或者0个
    if (!(t.parameter_num == 1 || t.parameter_num == 0)){
        t.error_flag = ABNORMAL;
        strcpy(t.control_msg,"输入参数有误\n");
        ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }

    // 读取参数
    char parameter[1024] = {0};
    splitParameter(t,t.parameter_num,parameter);

    // 检测cd参数是否为0个，若为0个直接返回家目录
    char virtual_path[1024] = {0};
    if (t.parameter_num == 0 || (t.parameter_num == 1 && strcmp(parameter,"\n") == 0)){
        bzero(t.control_msg,sizeof(t.control_msg));
        strcpy(t.control_msg,"/");
        t.current_layers = 0;
        t.path_length = strlen(t.control_msg);
        t.error_flag = NORMAL;

        printf("cd #%s#\n",t.control_msg);
        
        ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return 0;
    }

    // 读取虚拟路径
    splitParameter(t,0,virtual_path);

    // 取出虚拟路径中的最后的家目录
    if (virtual_path[strlen(virtual_path) - 1] == '/'){
        virtual_path[strlen(virtual_path) - 1] = 0;
    } 

    // 读取当前层数
    int current_layers = t.current_layers;

    // 将参数最后一个换行符去除
    parameter[strlen(parameter) - 1] = 0;

    // 判断参数最后是否有/，没有就补
    if (parameter[strlen(parameter) - 1] != '/'){
        parameter[strlen(parameter)] = '/'; 
    }

    // 判断是否需要在根目录下寻找,若在根目录下寻找则需要重构虚拟路径
    if(parameter[0] == '/'){
        bzero(virtual_path,sizeof(virtual_path));
        current_layers = 0;
    }

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
                        if (current_layers == 0){
                            strcat(t.control_msg,"/");
                        }
                        t.error_flag = NORMAL;
                        t.current_layers = current_layers;
                        t.path_length = strlen(t.control_msg);
                        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                        ERROR_CHECK(ret,-1,"send");
                        return 0;
                    }
                    else{
                        puts("139");
                        t.error_flag = NORMAL;
                        bzero(t.control_msg,sizeof(t.control_msg));
                        strcat(t.control_msg,"/");
                        t.path_length = strlen(t.control_msg);
                        t.current_layers = 0;
                        int ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                        ERROR_CHECK(ret,-1,"send");
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
            rollbackPath(virtual_path);
            current_layers--;
            if (current_layers < 0){
                t.error_flag = ABNORMAL;
                strcpy(t.control_msg,"输入有误");
                ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
                ERROR_CHECK(ret,-1,"send");
                return -1;
            }
        }
        // 若为文件名，复合要求就进行追加
        else {
            bool flag = isExistDir(mysql,t,virtual_path,tmp_parameter);
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
        }
    }
    // 将拼接好的路径发送给客户端
    bzero(t.control_msg,sizeof(t.control_msg));
    memcpy(t.control_msg, virtual_path, sizeof(virtual_path));
    if (current_layers == 0){
        strcat(t.control_msg,"/");
    }
    t.error_flag = NORMAL;
    t.current_layers = current_layers;
    t.path_length = strlen(t.control_msg);
    ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
    ERROR_CHECK(ret,-1,"send");
    return 0;
}

