#include "header.h"
#include <bits/types/struct_osockaddr.h>
#include <mysql/mysql.h>

int deleteFile(char * file_path, MYSQL* mysql){

     // 删除 file_path 中末尾的换行符
    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0'; // 将换行符替换为字符串终止符
    }

    MYSQL_RES *res;

    char sql[4096] = { 0 };
    snprintf(sql,sizeof(sql),
             "UPDATE files set delete_flag = 1 where file_path = '%s'",
             file_path);
    printf("sql:#%s#\n",sql);
    
    if(mysql_query(mysql,sql)){
        printf("%s\n",mysql_error(mysql));
        return -1;
    }
        
    res = mysql_store_result(mysql);

    mysql_free_result(res);

    return 0;
}
