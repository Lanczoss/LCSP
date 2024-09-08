#include "header.h"

//从train自定义协议中取得参数
//第一个参数指自定义协议
//第二个参数指需要第几个参数
//第三个参数指字符数组缓冲区
int splitParameter(train_t t, int num, char *buf){

    //当前t.control_msg存放信息：zs/ 参数1 参数2

    //取出t.control_msg中的信息，并且要保持里面的信息内容不变
    char str[1024] = { 0 };

    //放入str中，在str中取出数据
    strcpy(str,t.control_msg);
    
    char *parma;
    //取出路径
    parma = strtok(str," ");
    
    //取出参数
    for(int i = 0;i < num;i++){
        parma = strtok(NULL, " ");
        ERROR_CHECK(parma, NULL, "strtok");
        printf("parma:%s\n",parma);
    }
    
    strcpy(buf,parma);

    return 0;
}


