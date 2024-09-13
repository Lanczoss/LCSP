#include "header.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

int deleteFile(train_t t, char *file_path, MYSQL *mysql) {
    // 删除 file_path 中末尾的换行符
    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0';  // 将换行符替换为字符串终止符
    }

    MYSQL_RES *res;
    MYSQL_ROW row;

    // 检测数据库中是否存在此文件夹
    char check_sql[4096] = {0};
    snprintf(check_sql, sizeof(check_sql),
             "SELECT id FROM files WHERE uid = %d AND file_path = '%s' AND delete_flag = 0",
             t.uid, file_path);
    LOG_INFO(check_sql);
    if (mysql_query(mysql, check_sql)) {
        fprintf(stderr, "检测文件夹存在性失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (res) {
        if (mysql_num_rows(res) == 0) {  // 表示文件夹不存在或已删除
            printf("文件夹不存在或已被标记为删除: %s\n", file_path);
            mysql_free_result(res);
            return -1;
        }
        mysql_free_result(res);
    } else {
        fprintf(stderr, "获取检查结果集失败: %s\n", mysql_error(mysql));
        return -1;
    }

    char sql[8192] = {0};

    // 删除子文件：查询子文件的 id
    snprintf(sql, sizeof(sql),
             "SELECT id FROM files WHERE file_path = '%s' AND delete_flag = 0 AND uid = %d",
             file_path, t.uid);
    LOG_INFO(sql);
    if (mysql_query(mysql, sql)) {
        fprintf(stderr, "查询子文件 ID 失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    int id = -1;  // 默认值为 -1，用于检测是否获取到有效 ID
    if (res) {
        while ((row = mysql_fetch_row(res)) != NULL) {
            id = atoi(row[0]);  // 获取第一个字段作为 id
        }
        mysql_free_result(res);
    } else {
        fprintf(stderr, "获取子文件 ID 结果集失败: %s\n", mysql_error(mysql));
        return -1;
    }

    if (id == -1) {
        printf("无法获取子文件 ID。\n");
        return -1;
    }

    // 更新子文件的 delete_flag
    bzero(sql, sizeof(sql));
    snprintf(sql, sizeof(sql),
             "UPDATE files AS f "
             "INNER JOIN (SELECT id FROM files WHERE file_path = '%s' AND uid = %d) AS subquery "
             "ON f.pid = subquery.id "
             "SET f.delete_flag = -1;",
             file_path, t.uid);
    LOG_INFO(sql);
    if (mysql_query(mysql, sql)) {
        printf("更新子文件失败: %s\n", mysql_error(mysql));
        return -1;
    }

    // 删除空文件夹
    bzero(sql, sizeof(sql));
    snprintf(sql, sizeof(sql),
             "UPDATE files SET delete_flag = -1 WHERE file_path = '%s'",
             file_path);
    LOG_INFO(sql);

    if (mysql_query(mysql, sql)) {
        printf("更新空文件夹失败: %s\n", mysql_error(mysql));
        return -1;
    }

    return 0;
}

