#include "header.h"
#include <mysql/mysql.h>

int reName(train_t t, int net_fd, MYSQL* mysql){
    
    MYSQL_RES *res;

    char file_old_name[1024] = { 0 };
    char file_old_path[1024] = { 0 };
    char file_new_name[1024] = { 0 };
    char file_new_path[1024] = { 0 };
    splitParameter(t, 1, file_old_name);
    
    splitParameter(t,2,file_new_name);
    file_new_name[strlen(file_new_name) - 1] = '\0';
    
    char temp[1024] = { 0 };
    strcpy(temp, t.control_msg);

    char *parma;
    parma = strtok(temp," ");
    strcpy(file_old_path, parma);
    strcpy(file_new_path, parma);
    strcat(file_old_path, file_old_name);
    strcat(file_new_path, file_new_name);
    printf("old_file_path:#%s#\n",file_old_path);
    printf("new_file_path,#%s#\n",file_new_path);
    
    //检测旧文件是否存在
    char check_sql[2048] = { 0 };
    snprintf(check_sql,sizeof(check_sql),
             "SELECT * FROM files WHERE file_path = '%s' and delete_flag = 0",file_old_path);
    printf("check_sql:#%s#\n",check_sql);
    mysql_query(mysql,check_sql);
    res = mysql_store_result(mysql);

    //判空
    if(res == NULL){
        char str[] = "该文件不存在!\n";
        send(net_fd,str,strlen(str),MSG_NOSIGNAL);
        return -1;
    }

    //检测新文件是否存在
    bzero(check_sql,sizeof(check_sql));
    snprintf(check_sql,sizeof(check_sql),
            "SELECT * FROM files WHERE file_path = '%s' and delete_flag = 0",file_new_path);
    printf("new_check_sql:#%s#\n",check_sql);
    mysql_query(mysql, check_sql);
    res = mysql_store_result(mysql);
    if(mysql_num_rows(res) > 0){
        char str[] = "新文件已存在!\n";
        send(net_fd,str,strlen(str),MSG_NOSIGNAL);
        return -1;
    }

    //开始正式改名
    char sql[4096] = { 0 };
    size_t len = strlen(file_old_path);
    // 如果字符串长度大于0且最后一个字符是换行符 '\n'
    if (len > 0 && file_old_path[len - 1] == '\n') {
        file_old_path[len - 1] = '\0'; // 将末尾的换行符替换为 '\0'
    }
    snprintf(sql,sizeof(sql),
            "UPDATE files SET file_name = '%s', file_path = '%s' where file_path = '%s'",
             file_new_name,file_new_path,file_old_path);
    printf("sql:#%s#\n",sql);

    if(mysql_query(mysql, sql)){
        printf("%s\n",mysql_error(mysql));
        return -1;
    }

    res = mysql_store_result(mysql);
    
    mysql_free_result(res);

    char str[] = "改名成功！\n";
    send(net_fd,str,strlen(str),MSG_NOSIGNAL);
    LOG_ERROR("rename_send");
    return 0;
}
