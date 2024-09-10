#include "header.h"

//根据用户名从数据库中获取uid的函数
int getUidMysql(char *user_name, MYSQL *mysql)
{
    char tmp[60] = {0};
    //根据用户名获取uid
    sprintf(tmp, "select user_id from users where user_name='%s'", user_name);
    mysql_query(mysql, tmp);
    printf("%s\n", mysql_error(mysql));
    MYSQL_RES *res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("%s\n", mysql_error(mysql));
        return -1;
    }
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)) != NULL)
    {
        mysql_free_result(res);
        return atoi(row[0]);
    }
    mysql_free_result(res);
    return 0;
}