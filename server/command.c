#include "header.h"
#include <asm-generic/errno-base.h>
#include <complex.h>
#include "command.h"

int cdCommand(train_t t, int net_fd){
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
                        if (current_layers == 0 && strlen(t.control_msg) != '/'){
                            strcat(t.control_msg,"/");
                        }
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
    if (current_layers == 0 && strlen(t.control_msg) != '/'){
        strcat(t.control_msg,"/");
    }
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

