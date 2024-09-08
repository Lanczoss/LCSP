#ifndef BAIDU_COMMAND_H
#define BAIDU_COMMAND_H


//提取一个字符串最后的文件名 eg: /a/b/c.txt -> c.txt
int extractFilename(const char *arg, char **file_name);

//在一个字符串中提取参数
int extractParameters(char *str, int num, char *arg);

#endif //BAIDU_COMMAND_H
