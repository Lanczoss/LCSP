#include "header.h"

//获取盐值的函数
//需要传入一个指向buf的空间和一个指定盐值长度
int getSaltValue(char *buf, int salt_length)
{
    strcpy(buf, SALT_PREFIX);
    // 初始化随机数生成器
    srand(time(NULL));
    // 用于生成随机盐值的字符集
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";
    for (size_t i = 3; i < salt_length + 3; i++) {
        int key = rand() % (sizeof(charset) - 1);
        buf[i] = charset[key];
    }
    buf[salt_length + 3] = '\0';  // 确保字符串结束
    return 0;
}

//获取散列的函数
//需要传入一个指向buf的空间
//buf返回类似"$6$EcGQ/umB$"
//一个盐值
//一个明文密码
int getHashValue(char *buf, char *salt, const char *password)
{
    getSaltValue(salt, SALT_LENGTH);
    char *encrypted = crypt(password, salt);
    ERROR_CHECK(encrypted, NULL, "散列计算失败");
    //printf("password = %s\n", password);
    //printf("salt = %s\n", salt);
    //printf("encrypted = %s\n", encrypted);
    strcpy(buf, encrypted);

    //打印
    //printf("password = %s\n", buf);
    return 0;
}