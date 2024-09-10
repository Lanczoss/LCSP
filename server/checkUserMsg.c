#include "header.h"

//登录注册时尝试判断是否存在该用户
//第一版遍历目录
//第三版查找MySQL表
int checkUserMsg(const char *user_name, MYSQL *mysql)
{
    //登录行为/注册行为
    //需要从users表中的user_name对比
    mysql_query(mysql, "select user_name from users");
    printf("%s\n", mysql_error(mysql));
    MYSQL_RES *res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("%s\n", mysql_error(mysql));
    }
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)) != NULL)
    {
        if(strcmp(row[0], user_name) == 0)
        {
            //有这个用户名
            //如果是注册行为
            //注册失败、登录成功
            //返回-1
            mysql_free_result(res);
            return -1;
        }
    }
    //没有这个用户名
    //登录失败
    //注册成功
    //返回0
    mysql_free_result(res);
    return 0;
}

//验证密码
//需要查找对应用户的盐值来获取hash值
int checkPassword(const char *user_name, const char *password, MYSQL *mysql)
{
    //根据用户名查找盐值
    //从而得出加密后的encrypted_password
    //最后进行比较
    char salt[32] = {0};
    char tmp[128] = {0};
    char encrypted_password[256] = {0};

    //根据用户名获取盐值
    sprintf(tmp, "select salt, passwd from users where user_name='%s'", user_name);
    mysql_query(mysql, tmp);
    printf("%s\n", mysql_error(mysql));
    MYSQL_RES *res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("%s\n", mysql_error(mysql));
    }
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)) != NULL)
    {
        strcpy(salt, row[0]);
        //根据盐值计算散列
        char *encrypted = crypt(password, salt);
        if (encrypted == NULL) {
            perror("计算失败 \n");
            return 0;
        }
        strcpy(encrypted_password, encrypted);

        //根据用户名获取散列值
        if(strcmp(row[1], encrypted_password) == 0)
        {
            //密码正确
            mysql_free_result(res);
            return 0;
        }
        //密码错误
        mysql_free_result(res);
        return -1;
    }
    return -1;
}

