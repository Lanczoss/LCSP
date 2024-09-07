#include "header.h"

//读配置文件                                   
// 读取配置文件并获取指定参数的值  
// key: 指向要查找的配置项的键（字符串）  
// value: 指向用于存储找到的配置项值的缓冲区  
int getParameter(void *key, void *value)  
{  
    // 打开配置文件  
    FILE *file = fopen("config.ini", "r");  
    ERROR_CHECK(file, NULL, "fopen"); // 检查文件打开是否成功  

    // 循环读取配置文件的每一行  
    while(1)  
    {  
        char line[100]; // 用于存储读取的行  
        memset(line, 0, sizeof(line)); // 清空行缓冲区  

        // 读一行数据  
        char *res = fgets(line, sizeof(line), file);  
        if(res == NULL) // 如果没有读取到数据  
        {  
            char buf[] = "没有要找到的内容 \n"; // 提示信息  
            memcpy(value, buf, strlen(buf)); // 将提示信息复制到value中  
            return -1; // 返回-1表示未找到内容  
        }   

        // 处理数据  
        char *line_key = strtok(line, "="); // 以'='为分隔符获取键  

        // 比较传入的key与当前行的key是否相同  
        if(strcmp((char *)key, line_key) == 0)  
        {  
            char *line_value = strtok(NULL, "="); // 获取对应的值  
            memcpy((char *)value, line_value, strlen(line_value)); // 将值复制到value中  
            return 0; // 返回0表示成功找到并复制值  
        }  
    }  

    return 0; // 默认返回0  
}
