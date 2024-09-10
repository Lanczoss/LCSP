#include "header.h"

//将得到的用户名、盐值和密码插入到users表中
//明文密码需要盐值加密
//明文密码还需要hash值
int registerInsertMysql(const char *user_name, const char *password, MYSQL *mysql)
{
    //保存类似"$6$EcGQ/umB$KkxG5RZFxRR2TYY0..."的信息
    char encrypted_pwd[512] = {0};
    //保存盐值
    char salt[32] = {0};
    getHashValue(encrypted_pwd, salt, password);
    //向数据库中插入盐值和encrypted_pwd
    char tmp[1024] = {0};
    //向users表插入注册信息
    sprintf(tmp, "insert into users (user_id, user_name, salt, passwd, create_time, update_time) values (NULL, '%s', '%s', '%s', now(), now())",
            user_name,
            salt,
            encrypted_pwd
        );
    if(mysql_query(mysql, tmp))
    {
        printf("%s\n", mysql_error(mysql));
    }
    //printf("%s\n", tmp);

    bzero(tmp, sizeof(tmp));
    //向files表插入注册信息
    sprintf(tmp, "insert into files (id, file_name, uid, pid, file_path, file_type, hash, create_time, update_time, delete_flag) values (NULL, '/', %d, -1, '/', 1, NULL, now(), now(), 0)",
            getUidMysql(user_name, mysql)
        );
    if(mysql_query(mysql, tmp))
    {
        printf("%s\n", mysql_error(mysql));
    }
    //printf("%s\n", tmp);
    //printf("插入成功\n");
    return 0;
}