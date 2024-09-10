#include "header.h"

//子线程连接数据库函数
int connectMysql(MYSQL **mysql)
{
    *mysql = mysql_init(NULL);
    printf("%s\n", mysql_error(*mysql));

    //读取配置文件信息
    char user[1024] = {0};
    char password[1024] = {0};
    char database[512] = {0};
    int ret = getParameter((void*)"mysql_user", (void*)user);
    ERROR_CHECK(ret, -1, "config不存在mysql_user");
    user[strlen(user) - 1] = '\0';
    ret = getParameter((void*)"mysql_password", (void*)password);
    ERROR_CHECK(ret, -1, "config不存在mysql_password");
    password[strlen(password) - 1] = '\0';
    ret = getParameter((void*)"mysql_database", (void*)database);
    ERROR_CHECK(ret, -1, "config不存在mysql_database");
    database[strlen(database) - 1] = '\0';
    printf("mysql\nuser: %s\npassword: %s\ndatabase: %s\n", user, password, database);
    
    //尝试连接数据库
    MYSQL *mret = mysql_real_connect(*mysql, "localhost", user, password, database, 0, NULL, 0);
    printf("%s\n", mysql_error(*mysql));
    if(mret == NULL)
    {
        return -1;
    }
    printf("database connected.\n");
    return 0;
}
