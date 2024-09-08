#include "header.h"
#include <string.h>

//通过控制命令得到对应的枚举类型
//str:传入的控制命令 eg:cd,ls
int getEnum(char *str){
    switch(str[0]){
    case 'l':return LS;
    case 'c':return CD;
    case 'r':return (str[1] == 'm') ? RM:REMOVE;
    case 'm':return MKDIR; 
    case 'p':return (str[1 ]== 'u') ? PUTS:PWD;
    case 'g':return GETS;
    default :
             printf("命令输入错误");
             return -1;
             break;
    }
    return 0;
}

//分割从键盘输入的命令
//train_t:需要初始化的协议
//buf：存放命令的buf数组
int splitCommand(train_t *t, char *buf){
    
    bzero(t->control_msg,sizeof(t->control_msg));
    bzero(&t->parameter_num,sizeof(t->parameter_num));
    printf("buf:%s \n",buf);
    char str[1024] = { 0 };
    strcpy(str,buf);

    //切割出文件路径
    char *path;
    path = strtok(str," ");

    //将路径存放到自定义协议
    strncpy(t->control_msg, path, strlen(path));
    t->control_msg[strlen(t->control_msg)] = ' ';

    //切割从标准输入输入的命令
    char *parma;
    //将str改成NULL
    parma = strtok(NULL, " ");
    t->command = getEnum(parma);

    //判断有几个参数
    while((parma = strtok(NULL, " ")) != NULL){
        t->parameter_num++;
        strncat(t->control_msg,parma,strlen(parma));
        t->control_msg[strlen(t->control_msg)] = ' ';
        printf("parma:%s\n",parma);
    }

    printf("t-control_msg:%s\n",t->control_msg);
    return 0;
}

