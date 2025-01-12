#include "header.h"

//子线程连接数据库函数
int connectMysql(MYSQL **mysql)
{
    *mysql = mysql_init(NULL);
    if(mysql == NULL)
    {
        printf("%s\n", mysql_error(*mysql));
        return -1;
    }
    //读取配置文件信息
    char user[1024] = {0};
    char password[1024] = {0};
    char database[512] = {0};
    char log[4096] = {0};
    int ret = getParameter((void*)"mysql_user", (void*)user);
    ERROR_CHECK(ret, -1, "config have no mysql_user.");
    user[strlen(user) - 1] = '\0';
    ret = getParameter((void*)"mysql_password", (void*)password);
    ERROR_CHECK(ret, -1, "config have no mysql_password");
    password[strlen(password) - 1] = '\0';
    ret = getParameter((void*)"mysql_database", (void*)database);
    ERROR_CHECK(ret, -1, "config have no mysql_database");
    database[strlen(database) - 1] = '\0';
    sprintf(log, "mysql\nuser: %s\npassword: %s\ndatabase: %s\n", user, password, database);
    LOG_INFO(log);
    
    //尝试连接数据库
    MYSQL *mret = mysql_real_connect(*mysql, "localhost", user, password, database, 0, NULL, 0);
    if(mret == NULL)
    {
        printf("%s\n", mysql_error(*mysql));
        return -1;
    }
    printf("database connected.\n");
    return 0;
}
