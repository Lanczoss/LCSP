#ifndef BAIDU_GONGJU_H
#define BAIDU_GONGJU_H

#include <mysql/mysql.h>
#include "header.h"

//提取一个字符串最后的文件名 eg: /a/b/c.txt -> c.txt
int extractFilename(const char *arg, char *filename, int max_len);

//在一个字符串中提取参数
int extractParameters(const char *str, int num, char *parameter, int max_len);

//提取第一个/后面的字符
int extract_path_after_first_slash(const char *arg, char *result, int max_len);

int extractBasePath(const char *path, char *base_path, int max_len);

//计算文件hash值
int getFileHash(const char *filename, char *hash);

#endif //BAIDU_GONGJU_H
