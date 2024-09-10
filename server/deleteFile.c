#include "header.h"
#include <bits/types/struct_osockaddr.h>
#include <mysql/mysql.h>

int deleteFile(train_t t, char * file_path, MYSQL* mysql){

    // 删除 file_path 中末尾的换行符
    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0'; // 将换行符替换为字符串终止符
    }

    MYSQL_RES *res;

    //检测数据库存不存在此文件夹
    char check_sql[4096] = { 0 };
    snprintf(check_sql, sizeof(check_sql),
             "SELECT id FROM files WHERE uid = %d AND file_path = '%s'AND delete_flag = 1 ",
             t.uid, file_path);
    printf("check_sql:#%s#\n",check_sql);
    if (mysql_query(mysql, check_sql)) {
        fprintf(stderr, "检测文件夹存在性失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (res) {
        if (mysql_num_rows(res) > 0) {
            printf("文件夹不存在: %s\n", file_path);
            mysql_free_result(res);
            return -1;  // 返回错误代码，表示文件夹已存在
        }
        mysql_free_result(res);
    } else {
        fprintf(stderr, "获取检查结果集失败: %s\n", mysql_error(mysql));
        return -1;
    }

    char sql[8192] = { 0 };

    //删除子文件
    //查询改文件的id
    strcpy(t.control_msg, file_path);
    int id = getFileId(t,mysql);
    //检错
    if (id == -1) {
        printf("无法获取文件 ID。\n");
        return -1;
    }
    printf("file_path:id = %d\n",id);
    snprintf(sql, sizeof(sql),
             "UPDATE files AS f "
             "INNER JOIN (SELECT id FROM files WHERE file_path = '%s' AND uid = %d) AS subquery "
             "ON f.pid = subquery.id "
             "SET f.delete_flag = 1;",
             file_path, t.uid);
    printf("son_sql:%s\n",sql);

    if(mysql_query(mysql,sql)){
        printf("%s\n",mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);

    //删除空文件夹
    bzero(sql,sizeof(sql));
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
