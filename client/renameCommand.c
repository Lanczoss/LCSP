#include "header.h"
#define MAX_PATH_LENGTH 1024
#define MAX_SQL_LENGTH 4096

int renameCommand(train_t t, int net_fd, MYSQL* mysql) {
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    char catalogue_name[MAX_SQL_LENGTH] = { 0 };  // 目录名路径
    char *cata_path;
    char temp[MAX_PATH_LENGTH] = { 0 };

    strncpy(temp, t.control_msg, sizeof(temp) - 1);
    cata_path = strtok(temp, " ");
    if (cata_path) {
        strncpy(catalogue_name, cata_path, sizeof(catalogue_name) - 1);
    }

    // 确保路径长度合理，避免缓冲区溢出
    if (strlen(catalogue_name) > 1000) {
        fprintf(stderr, "目录名路径过长！\n");
        return -1;
    }

    char new_path[MAX_PATH_LENGTH] = { 0 };
    strncpy(new_path, cata_path, sizeof(new_path) - 1);

    char parma[MAX_PATH_LENGTH] = { 0 };
    // 获取旧文件夹路径
    splitParameter(t, 1, parma);
    strncat(catalogue_name, parma, sizeof(catalogue_name) - strlen(catalogue_name) - 1);

    // 获取新文件夹路径
    char new_dir_name[MAX_PATH_LENGTH] = { 0 };
    splitParameter(t, 2, parma);
    splitParameter(t, 2, new_dir_name);
    
    // 去掉新文件夹名称末尾的换行
    size_t len = strlen(new_dir_name);
    if (len > 0 && new_dir_name[len - 1] == '\n') {
        new_dir_name[len - 1] = '\0';
    }
    strncat(new_path, parma, sizeof(new_path) - strlen(new_path) - 1);

    // 去掉末尾的换行
    len = strlen(new_path);
    if (len > 0 && new_path[len - 1] == '\n') {
        new_path[len - 1] = '\0';
    }

    len = strlen(catalogue_name);
    if (len > 0 && catalogue_name[len - 1] == '\n') {
        catalogue_name[len - 1] = '\0';
    }    

    // 查询旧目录名是否存在
    char check_sql[MAX_SQL_LENGTH] = { 0 };
    if (snprintf(check_sql, sizeof(check_sql),
             "SELECT * FROM files WHERE file_path = '%s' AND delete_flag = 0 AND uid = %d", 
             catalogue_name, t.uid) >= sizeof(check_sql)) {
        fprintf(stderr, "SQL查询过长，可能导致截断！\n");
        return -1;
    }

    if (mysql_query(mysql, check_sql)) {
        fprintf(stderr, "查询旧目录名失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (res && mysql_num_rows(res) == 0) {
        printf("旧文件夹名称不存在！\n");
        char str[1024] = "旧文件夹名称不存在！\n";
        send(net_fd, str, strlen(str), MSG_NOSIGNAL);
        mysql_free_result(res);
        return -1;
    }
    mysql_free_result(res);

    // 查询新目录名是否存在
    bzero(check_sql, sizeof(check_sql));
    if (snprintf(check_sql, sizeof(check_sql),
             "SELECT * FROM files WHERE file_path = '%s' AND delete_flag = 0 AND uid = %d", 
             new_path, t.uid) >= sizeof(check_sql)) {
        fprintf(stderr, "SQL查询过长，可能导致截断！\n");
        return -1;
    }

    if (mysql_query(mysql, check_sql)) {
        fprintf(stderr, "查询新目录名失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (res && mysql_num_rows(res) > 0) {
        printf("新文件夹名称已存在！\n");
        char str[1024] = "新文件夹名称已存在！\n";
        send(net_fd, str, strlen(str), MSG_NOSIGNAL);
        mysql_free_result(res);
        return -1;
    }
    mysql_free_result(res);

    // 获取目录名 ID
    bzero(t.control_msg, sizeof(t.control_msg));
    strncpy(t.control_msg, catalogue_name, sizeof(t.control_msg) - 1);
    int pid = getFileId(t, mysql);

    // 查询子文件名
    char query_sql[MAX_SQL_LENGTH] = { 0 };
    if (snprintf(query_sql, sizeof(query_sql),
             "SELECT id, file_name FROM files WHERE pid = %d AND uid = %d AND file_type = 2", 
             pid, t.uid) >= sizeof(query_sql)) {
        fprintf(stderr, "SQL查询过长，可能导致截断！\n");
        return -1;
    }

    if (mysql_query(mysql, query_sql)) {
        fprintf(stderr, "查询子文件名失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (!res) {
        fprintf(stderr, "获取子文件名结果失败: %s\n", mysql_error(mysql));
        return -1;
    }

    while ((row = mysql_fetch_row(res))) {
        int id = atoi(row[0]);
        char file_name[MAX_PATH_LENGTH] = { 0 };
        strncpy(file_name, row[1], sizeof(file_name) - 1);
        
        // 修剪路径
        int count = 0;
        for (char *p = new_path; *p && (*p != '\n'); ++p) {
            if (*p == '/' && ++count == 2) {
                *p = '\0';
                break;
            }
        }

        // 拼接子文件路径
        len = strlen(new_path);
        if (len > 0 && new_path[len - 1] != '/') {
            if (len < sizeof(new_path) - 1) {
                new_path[len] = '/';
                new_path[len + 1] = '\0';
            }
        }
        strncat(new_path, file_name, sizeof(new_path) - strlen(new_path) - 1);

        // 修改子文件路径
        char update_sql[MAX_PATH_LENGTH] = { 0 };
        if (snprintf(update_sql, sizeof(update_sql),
                 "UPDATE files SET file_path = '%s' WHERE id = %d AND uid = %d", 
                 new_path, id, t.uid) >= sizeof(update_sql)) {
            fprintf(stderr, "SQL更新语句过长，可能导致截断！\n");
            mysql_free_result(res);
            return -1;
        }

        if (mysql_query(mysql, update_sql)) {
            fprintf(stderr, "更新子文件路径失败: %s\n", mysql_error(mysql));
            mysql_free_result(res);
            return -1;
        }
    }
    mysql_free_result(res);

    // 修改目录名路径
    char update_sql[MAX_PATH_LENGTH] = { 0 };
    // 修剪新文件名以后的内容
    int count = 0;
    for (char *p = new_path; *p && (*p != '\n'); ++p) {
        if (*p == '/' && ++count == 2) {
            *p = '\0';
            break;
        }
    }
    if (snprintf(update_sql, sizeof(update_sql),
             "UPDATE files SET file_path = '%s',file_name = '%s' WHERE id = %d AND uid = %d", 
             new_path, new_dir_name, pid, t.uid) >= sizeof(update_sql)) {
        fprintf(stderr, "SQL更新语句过长，可能导致截断！\n");
        return -1;
    }

    if (mysql_query(mysql, update_sql)) {
        fprintf(stderr, "更新目录名路径失败: %s\n", mysql_error(mysql));
        return -1;
    }

    char str[] = "改名成功！\n";
    send(net_fd, str, strlen(str), MSG_NOSIGNAL);

    return 0;
}

