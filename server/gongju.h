#ifndef BAIDU_GONGJU_H
#define BAIDU_GONGJU_H

//#include <mysql/mysql.h>
#include "header.h"

//提取一个字符串最后的文件名 eg: /a/b/c.txt -> c.txt
int extractFilename(const char *arg, char *filename, int max_len);

//在一个字符串中提取参数
int extractParameters(const char *str, int num, char *parameter, int max_len);

//提取第一个/后面的字符
int extract_path_after_first_slash(const char *arg, char *result, int max_len);

int extractBasePath(const char *path, char *base_path, int max_len);

//判断数据库中文件是否存在
int judgeFileExist(MYSQL *mysql, char *filename, char *hash);

//计算文件hash值
int getFileHash(const char *filename, char *hash);

//hash是否相等
int judgeFileHash(MYSQL *mysql, const char *str, train_t t);

//下载文件插入数据库
int uploadDatabase(MYSQL *mysql, char *filename, int uid, int pid, char *filepath, char *hash);

//查询父目录id
int queryPid(MYSQL *mysql, const char *filepath, train_t t, int *pid);

//查询删除标记位
int queryDeleteMark(MYSQL *mysql, const char *filename, const train_t t, const char *filepath, const char *hash,
                    int *delete_flag);

//修改删除标记位
int modifyDeleteMark(MYSQL *mysql, const char *filename, const train_t t, const char *filepath, const char *hash);

#endif //BAIDU_GONGJU_H
