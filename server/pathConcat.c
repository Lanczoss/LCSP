#include "header.h"
#include <string.h>

//路劲的拼接
//t:传过来的自定义协议
//real_path:需要 返回的真实文件路径
int pathConcat(train_t t, char *real_path){

    //初始化数组和几倍呢路径
    char base_path[1024] = BASE_PATH;
    char vir_path[1024] = { 0 };

    //TO DO:修改代码，不要删除原来的客户端路径
    //DONE
    strncpy(vir_path,t.control_msg,t.path_length);
    vir_path[t.path_length] = '\0';

    if(vir_path[0] != '\0'){
        strncat(base_path,vir_path,sizeof(base_path)-strlen(base_path)-1);
    }

    strncpy(real_path, base_path, 1024 - 1);
    real_path[1024 -1] = '\0';  //确保real_path以'\0'结尾。
    
    return 0;
}
