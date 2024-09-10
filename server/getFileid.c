#include "header.h"
#include <mysql/mysql.h>

int getFileid(train_t t){

    char file_path[1024] = { 0 };
    strcpy(file_path,t.control_msg);
    printf("#%s#\n",file_path);

    //连接数据库准备工作
    MYSQL * conn;
    MYSQL_RES *result;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    //获取连接数据库参数
    char user[1024]  = { 0 };
    int ret = getParameter((void *)"user",(void *)user);
    ERROR_CHECK(ret, -1, "getParameter");

    // 去掉 user 参数末尾的换行符
    size_t user_len = strcspn(user, "\n");
    if (user_len < strlen(user)) {
        user[user_len] = '\0';
    }

    char passwd[1024] = { 0 };
    ret = getParameter((void *)"passwd",(void*)passwd);
    ERROR_CHECK(ret, -1, "getParameter");
    // 去掉 passwd 参数末尾的换行符
    size_t passwd_len = strcspn(passwd, "\n");
    if (passwd_len < strlen(passwd)) {
        passwd[passwd_len] = '\0';
    }

    char database[1024] = { 0 };
    ret = getParameter((void*)"database",(void*)database);
    ERROR_CHECK(ret, -1, "getParameter");
    // 去掉 database 参数末尾的换行符
    size_t database_len = strcspn(database, "\n");
    if (database_len < strlen(database)) {
        database[database_len] = '\0';
    }
    
    //连接数据库
    if(!mysql_real_connect(conn, "localhost", (char*)user, (char*)passwd, (char*)database, 0, NULL, 0)){
        printf("数据库连接错误");
        printf("%s\n",mysql_error(conn));
        return -1;
    }
        
    char sql[4096] = { 0 };
    snprintf(sql, sizeof(sql),
             "select id from files where file_path = '%s' and uid = 1",file_path);
    printf("#%s#\n",sql);

    if(mysql_query(conn,sql)){
        printf("%s\n",mysql_error(conn));
        return -1;
    }

    result = mysql_store_result(conn);

    
    int id = 0;

    while((row = mysql_fetch_row(result))){
        for(int i = 0;i < mysql_num_fields(result);i++){
            if(row[0] != NULL){
                //返回结果不为空
                id = atoi(row[0]);
            }else{
                return -1;
            }
            printf("#%s#\n",row[0]);
        }
    }

    mysql_free_result(result);
    mysql_close(conn);

    return id;
}
