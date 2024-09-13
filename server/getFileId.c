#include "header.h"
int getFileId(train_t t, MYSQL *mysql){

    MYSQL_ROW row;
    MYSQL_RES *result;
    char query[2048] = { 0 };   // 存储sql查询
    char file_path[1024] = { 0 };  // 存储用户提供的file_path
    int uid;              // 用户ID
    int file_id = -1;     // 最终找到的文件或目录的ID

    // 从train_t 获取pid和file_path
    uid = t.uid;

    // 拼接真实路径

    splitParameter(t, 0, file_path);
    // TODO:打印调试信息

    //使用 snprintf 拼接查询语句，查找用户文件目录id

    snprintf(query, sizeof(query),
             "select id from files where uid = %d and file_path = '%s'",uid, file_path);
    
    // 执行查询语句以查询根目录
    if(mysql_query(mysql, query)){
        fprintf(stderr, "Query filed : %s \n ", mysql_error(mysql));
        return -1;
    }

    // 处理查询结果
    result = mysql_store_result(mysql);
    if (result == NULL) {
        fprintf(stderr, "Failed to get resultult set: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        return -1;
    }

    // 验证行数
    my_ulonglong num_rows = mysql_num_rows(result);
    if(num_rows == 0){
        printf("未找到任何行 \n");
    }else{
        printf("行数: %lu\n", num_rows);
    }

    // 获取列数
    if((row = mysql_fetch_row(result)) != NULL){
        file_id = atoi((row[0]));
    }else{
        printf("没有匹配的uid=%d\n",uid);
    }

    // 释放查询结果集
    mysql_free_result(result);
    
    // 返回查找到的文件ID或-1(未找到)
    return file_id;

}

