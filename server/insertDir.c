#include "header.h"
#include <mysql/mysql.h>

int insertDir(train_t t, char * real_path, char * dirname,MYSQL*mysql){

    
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    //获取uid
    int uid = deCodeToken(t.token);


    //获取pid
    int pid = 0;
    char sql_pid[1024] = { 0 };
    snprintf(sql_pid, sizeof(sql_pid), 

              "SELECT id FROM files WHERE file_path = '%s' AND uid = %d", real_path,uid);
    mysql_query(mysql,sql_pid);
    
    printf("sql_pid:%s\n",sql_pid);
    res = mysql_store_result(mysql);
    
    while((row = mysql_fetch_row(res))){
        for(int i = 0;i < mysql_num_fields(res);i++){        //获取pid
            pid = atoi(row[i]);
        }
    }
    printf("pid:#%d#\n",pid);
    printf("新文件已存在!\n");

    //判断路径是否为跟或者目录
    //以路径结尾是否为"/"区分
    if(real_path[strlen(real_path) - 1] != '/'){
        //末尾添加'/' 
        strcat(real_path, "/");
    }

    char file_path[1024] = { 0 };
    strcpy(file_path, real_path);
    strcat(file_path,dirname);
    
    //设置字符编码
    mysql_set_character_set(mysql, "utf8mb4");

    //检测数据库存不存在此文件夹
    char check_sql[4096] = { 0 };
    snprintf(check_sql, sizeof(check_sql),
             "SELECT id FROM files WHERE uid = %d AND file_path = '%s' AND delete_flag = 0",
             uid, file_path);

    if (mysql_query(mysql, check_sql)) {
        fprintf(stderr, "检测文件夹存在性失败: %s\n", mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    if (res) {
        if (mysql_num_rows(res) > 0) {
            printf("文件夹已存在: %s\n", file_path);
            mysql_free_result(res);
            return -1;  // 返回错误代码，表示文件夹已存在
        }
        mysql_free_result(res);
    } else {
        fprintf(stderr, "获取检查结果集失败: %s\n", mysql_error(mysql));
        return -1;
    }


    bzero(res,sizeof(MYSQL*));

    //拼接SQL语句
    char sql[4096] = { 0 };
    bzero(sql,sizeof(sql));
    snprintf(sql, sizeof(sql),
             "INSERT INTO files (file_name,uid,pid,file_path,file_type,create_time,update_time)"
             "VALUES ('%s', %d, %d, '%s', 1, now(),now())",
             dirname, uid, pid, file_path);

    if(mysql_query(mysql,sql)){
        printf("%s\n",mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);

    mysql_free_result(res);

    return 0;
}
