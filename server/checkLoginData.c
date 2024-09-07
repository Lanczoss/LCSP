#include "header.h"

//登录注册时尝试判断是否存在该用户
//第一版先遍历目录
int doesHaveUser(train_t t)
{
    char user_name[1024] = {0};
    //这里清除如zs/后面的斜杆
    strncpy(user_name, t.control_msg, t.path_length - 1);
    DIR *dir = opendir(BASE_PATH);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, user_name) == 0) {
            closedir(dir);
            return 0; //找到了用户的文件夹
        }
    }
    closedir(dir);
    return -1; // 用户的文件夹不存在
}

//注册时尝试创建用户根目录的函数
int createUser(train_t t)
{
    char user_path[2048] = {0};
    strcpy(user_path, BASE_PATH);
    strcat(user_path, "/");
    strcat(user_path, t.control_msg);
    int ret = mkdir(user_path, 0777);
    return ret;
}
