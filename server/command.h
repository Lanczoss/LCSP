
#ifndef BAIDU_COMMAND_H
#define BAIDU_COMMAND_H
// 判断之是否存在文件夹
// 第一个参数：路径名
// 第二个参数：路径名或者文件名
// 第三个参数：标志位0为文件，1为文件夹
// 返回值：true表示有，false表示没有
bool isExistFileOrDir(char *path, char *name, int flag);


// 回退真实路径以及虚拟路径
// 第一个参数：虚拟路径
// 第二个参数：真实路径
// 返回值：0表示正常，其他表示异常
int rollbackPath(char *virtual_path, char *real_path);

//提取一个字符串最后的文件名 eg: /a/b/c.txt -> c.txt
int extractFilename(const char *arg, char **file_name);

//在一个字符串中提取参数
int extractParameters(char *str, int num, char *arg);


#endif //BAIDU_COMMAND_H
