#ifndef BAIDU_GONGJU_H
#define BAIDU_GONGJU_H

//提取一个字符串最后的文件名 eg: /a/b/c.txt -> c.txt
char* extractFilename(const char *arg);

//在一个字符串中提取参数
char* extractParameters(const char *str, int num);

//提取第一个/后面的字符
char* extract_path_after_first_slash(const char *arg);

#endif //BAIDU_GONGJU_H
