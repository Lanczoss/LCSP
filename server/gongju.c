#include "gongju.h"
#include <string.h>

/**
  ******************************************************************************
  * 功能: 获取文件名
  * 参数: arg -> 输入字符串
  *        filename -> 输出文件名的缓冲区
  *        max_len -> 文件名缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extractFilename(const char *arg, char *filename, int max_len) {
    // 找到最后一个/
    const char *last_slash = strrchr(arg, '/');

    // 如果没有/，找到最后一个空格
    if (last_slash == NULL) {
        last_slash = strrchr(arg, ' ');
    }

    // 如果没有找到任何分隔符，则返回 -1
    if (last_slash == NULL) {
        return -1;
    }

    // 获取文件名
    strncpy(filename, last_slash + 1, max_len - 1);
    filename[max_len - 1] = '\0'; // 确保以 '\0' 结尾

    return 0; // 成功
}

/**
  ******************************************************************************
  * 功能: 提取第 num 个参数
  * 参数: str -> 输入字符串
  *        num -> 目标子字符串的索引，从1开始
  *        parameter -> 输出子字符串的缓冲区
  *        max_len -> 子字符串缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extractParameters(const char *str, int num, char *parameter, int max_len) {
    if (str == NULL || num < 1) {
        return -1;
    }

    char temp[256] = {0};
    strcpy(temp, str);

    char *token;
    char *delimiters = " "; // 以空格作为第一个分隔符
    int count = 0;

    // 第一次分割，分割空格
    token = strtok(temp, delimiters);
    while (token != NULL) {
        count++;
        if (count == num) {
            // 找到目标子字符串
            strncpy(parameter, token, max_len - 1);
            parameter[max_len - 1] = '\0'; // 确保以 '\0' 结尾
            return 0; // 成功
        }
        token = strtok(NULL, delimiters);
    }

    return -1; // 未找到第 num 个子字符串
}

/**
  ******************************************************************************
  * 功能: 提取第一个 / 后面的字符
  * 参数: arg -> 输入字符串
  *        result -> 输出 / 后的字符串的缓冲区
  *        max_len -> 输出缓冲区的最大长度
  * 返回值: 返回 0 表示成功，返回 -1 表示失败
  ******************************************************************************
**/
int extract_path_after_first_slash(const char *arg, char *result, int max_len) {
    const char *slash = strchr(arg, '/'); // 查找第一个 '/'

    if (slash == NULL) {
        // 如果没有找到 '/'，则返回 -1
        return -1;
    }

    // 计算需要复制的字符数（包括结尾的 '\0'）
    size_t length = strlen(slash + 1); // 从 '/' 之后开始到字符串末尾的长度

    // 检查输出缓冲区的长度
    if (length >= max_len) {
        return -1; // 输出缓冲区太小
    }

    // 复制字符串（从 '/' 之后开始）
    strcpy(result, slash + 1);

    return 0; // 成功
}

/**
 * 功能: 从给定的路径字符串中提取出斜杠（`/`）前的部分
 * 参数: path -> 输入的路径字符串
 *        base_path -> 输出路径部分的缓冲区
 *        max_len -> 输出缓冲区的最大长度
 * 返回值: 返回 0 表示成功，返回 -1 表示失败
 */
int extractBasePath(const char *path, char *base_path, int max_len) {
    if (path == NULL) {
        return -1;
    }

    // 查找第一个 '/' 的位置
    const char *first_slash = strchr(path, '/');

    if (first_slash == NULL) {
        // 如果没有找到 '/'，返回原始字符串的副本
        strncpy(base_path, path, max_len - 1);
        base_path[max_len - 1] = '\0'; // 确保以 '\0' 结尾
        return 0; // 成功
    }

    // 计算需要的长度，包括第一个 '/'
    size_t length = first_slash - path + 1; // +1 保留 '/'

    // 检查输出缓冲区的长度
    if (length >= max_len) {
        return -1; // 输出缓冲区太小
    }

    // 复制字符串
    strncpy(base_path, path, length);
    base_path[length] = '\0'; // 确保以 '\0' 结尾

    return 0; // 成功
}

/**
  ******************************************************************************
  * 功能: 判断文件名以及 hash 是否存在
  * 参数: 参数:mysql 为数据库连接，filename 为文件名，hash 为文件的哈希值
  * 返回值: 正常返回 0，文件不存在返回 -1
  ******************************************************************************
**/
int judgeFileExist(MYSQL *mysql, char *filename, char *hash) {
    mysql_set_character_set(mysql, "utf8mb4");

    // SQL 查询语句
    const char *query = "SELECT file_name, hash FROM files";

    // 创建预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        fprintf(stderr, "Error in judgeFileExist() to mysql_stmt_init() failed\n");
        return -1;
    }

    // 准备 SQL 语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error in judgeFileExist() to mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error in judgeFileExist() to mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定查询结果
    MYSQL_BIND result_bind[2];
    char db_file_name[256];
    char db_hash[41]; // SHA-1 哈希值长度为 40 字符，加上终止符 '\0'
    memset(result_bind, 0, sizeof(result_bind));

    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = db_file_name;
    result_bind[0].buffer_length = sizeof(db_file_name);
    result_bind[0].is_null = 0;
    result_bind[0].length = 0;

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = db_hash;
    result_bind[1].buffer_length = sizeof(db_hash);
    result_bind[1].is_null = 0;
    result_bind[1].length = 0;

    if (mysql_stmt_bind_result(stmt, result_bind)) {
        fprintf(stderr, "Error in judgeFileExist() to mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 取出结果并对比
    int file_exists = 0;
    while (mysql_stmt_fetch(stmt) == 0) {
        if (strcmp(db_file_name, filename) == 0 && strcmp(db_hash, hash) == 0) {
            file_exists = 1;
            break;
        }
    }

    // 清理
    mysql_stmt_close(stmt);

    // 返回结果
    return file_exists ? 0 : -1;
}

/**
  ******************************************************************************
  * 功能:计算文件的哈希值
  * 参数:filename文件路径，hash哈希值
  * 返回值:正常返回0
  ******************************************************************************
**/
int getFileHash(const char *filename, char *hash) {
    // 构造命令
    char command[256];
    snprintf(command, sizeof(command), "sha1sum \"%s\"", filename);

    // 使用 popen 执行命令
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("popen");
        return -1;
    }

    // 读取命令输出
    if (fgets(hash, 41, pipe) == NULL) {
        perror("fgets");
        pclose(pipe);
        return -1;
    }

    // `sha1sum` 命令的输出格式为 "<hash> <filename>\n"，需要提取 hash 部分
    char *space = strchr(hash, ' ');
    if (space != NULL) {
        *space = '\0'; // 只保留哈希值部分
    }

    pclose(pipe);
    return 0;
}

/**
  ******************************************************************************
  * 功能: 判断hash值是否一致
  * 参数: mysql 为数据库连接，str 为hash值，t 为用户结构体
  * 返回值: 修改成功返回 0，错误返回 -1
  ******************************************************************************
**/
int judgeFileHash(MYSQL *mysql, const char *str, int uid) {
    int user_uid = uid;
    mysql_set_character_set(mysql, "utf8mb4");

    // SQL 查询语句，使用占位符
    const char *query = "SELECT hash FROM files WHERE uid = ?";

    // 创建预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    // 准备 SQL 语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定参数
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *) &user_uid;
    bind[0].is_null = 0;
    bind[0].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定查询结果
    MYSQL_BIND result_bind[1];
    char db_hash[41]; // SHA-1 哈希值长度为 40 字符，加上终止符 '\0'
    memset(result_bind, 0, sizeof(result_bind));

    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = db_hash;
    result_bind[0].buffer_length = sizeof(db_hash);
    result_bind[0].is_null = 0;
    result_bind[0].length = 0;

    if (mysql_stmt_bind_result(stmt, result_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 取出结果并对比
    int file_exists = 0;
    while (mysql_stmt_fetch(stmt) == 0) {
        if (strcmp(db_hash, str) == 0) {
            file_exists = 1;
            break;
        }
    }

    // 清理
    mysql_stmt_close(stmt);

    // 返回结果
    return file_exists ? 0 : -1;
}

/**
  ******************************************************************************
  * 功能:返回pid
  * 参数:mysql为数据库连接，filepath为文件路径，t参考头文件header，pid传入传出参数接收查到的id
  * 返回值:正常返回0
  ******************************************************************************
**/
int queryPid(MYSQL *mysql, const char *filepath, int uid, int *pid) {
    int user_id = uid;
    mysql_set_character_set(mysql, "utf8mb4");

    // SQL 查询语句，使用占位符
    const char *query = "SELECT id FROM files WHERE uid = ? AND file_path = ?";

    // 创建预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    // 准备 SQL 语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定参数
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    // 绑定 user_id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *) &user_id;
    bind[0].is_null = 0;
    bind[0].length = 0;

    // 绑定 filepath
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *) filepath;
    bind[1].buffer_length = strlen(filepath);
    bind[1].is_null = 0;
    bind[1].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定查询结果
    MYSQL_BIND result_bind;
    int result_pid;
    memset(&result_bind, 0, sizeof(result_bind));

    result_bind.buffer_type = MYSQL_TYPE_LONG;
    result_bind.buffer = (char *) &result_pid;
    result_bind.is_null = 0;
    result_bind.length = 0;

    if (mysql_stmt_bind_result(stmt, &result_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 取出结果
    if (mysql_stmt_fetch(stmt) == 0) {
        *pid = result_pid;
        printf("pid:%d\n", *pid); // 将结果存入传入的参数
    } else {
        fprintf(stderr, "mysql_stmt_fetch() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 清理
    mysql_stmt_close(stmt);

    return 0;
}

int uploadDatabase(MYSQL *mysql, char *filename, int uid, int pid, char *filepath, char *hash) {

    char query[1024] = {0};
    sprintf(query,
            "INSERT INTO files (id,file_name,uid,pid,file_path,file_type,hash,create_time,update_time,delete_flag)"
            " VALUES(NULL,'%s',%d,%d,'%s',2,'%s',now(),now(), 0)", filename, uid, pid, filepath, hash);

    if (mysql_query(mysql, query)) {
        fprintf(stderr, "Error: %s\n", mysql_error(mysql));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

/**
  ******************************************************************************
  * 功能: 查询文件的 delete 标记位
  * 参数: mysql 为数据库连接，filename 为文件名，t 为用户结构体，filepath 为文件路径，hash 为文件哈希值，delete 标记位的指针
  * 返回值: 查询成功返回 0，错误返回 -1
  ******************************************************************************
**/
int queryDeleteMark(MYSQL *mysql, const char *filename, const int uid, const char *filepath, const char *hash,
                    int *delete) {

    int user_id = uid;
    // 设置字符集
    mysql_set_character_set(mysql, "utf8mb4");

    // SQL 查询语句
    const char *query = "SELECT delete_flag FROM files WHERE file_name = ? AND uid = ? AND file_path = ? AND hash = ?";

    // 创建预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    // 准备 SQL 语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定参数
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));

    // 绑定 file_name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *) filename;
    bind[0].buffer_length = strlen(filename);
    bind[0].is_null = 0;
    bind[0].length = 0;

    // 绑定 uid
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *) &user_id;
    bind[1].is_null = 0;
    bind[1].length = 0;

    // 绑定 file_path
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *) filepath;
    bind[2].buffer_length = strlen(filepath);
    bind[2].is_null = 0;
    bind[2].length = 0;

    // 绑定 hash
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (char *) hash;
    bind[3].buffer_length = strlen(hash);
    bind[3].is_null = 0;
    bind[3].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定查询结果
    MYSQL_BIND result_bind;
    int delete_mark;
    memset(&result_bind, 0, sizeof(result_bind));

    result_bind.buffer_type = MYSQL_TYPE_LONG;
    result_bind.buffer = (char *) &delete_mark;
    result_bind.is_null = 0;
    result_bind.length = 0;

    if (mysql_stmt_bind_result(stmt, &result_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 取出结果
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == MYSQL_NO_DATA) {
        // 没有数据
        mysql_stmt_close(stmt);
        return -1;
    } else if (fetch_result != 0) {
        // 查询失败
        mysql_stmt_close(stmt);
        return -1;
    }

    // 设置 delete 标记位的值
    *delete = delete_mark;

    // 清理
    mysql_stmt_close(stmt);

    // 返回成功
    return 0;
}

/**
  ******************************************************************************
  * 功能: 修改文件的 delete 标记位为 0
  * 参数: mysql 为数据库连接，filename 为文件名，t 为用户结构体，filepath 为文件路径，hash 为文件哈希值
  * 返回值: 修改成功返回 0，错误返回 -1
  ******************************************************************************
**/
int modifyDeleteMark(MYSQL *mysql, const char *filename, const int uid, const char *filepath, const char *hash) {
    int user_uid=uid;
    // 设置字符集
    mysql_set_character_set(mysql, "utf8mb4");

    // SQL 更新语句
    const char *query = "UPDATE files SET delete_flag = 0 and update_time = now() WHERE file_name = ? AND uid = ? AND file_path = ? AND hash = ?";

    // 创建预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    // 准备 SQL 语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 绑定参数
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));

    // 绑定 file_name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *) filename;
    bind[0].buffer_length = strlen(filename);
    bind[0].is_null = 0;
    bind[0].length = 0;

    // 绑定 uid
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *) &user_uid;
    bind[1].is_null = 0;
    bind[1].length = 0;

    // 绑定 file_path
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *) filepath;
    bind[2].buffer_length = strlen(filepath);
    bind[2].is_null = 0;
    bind[2].length = 0;

    // 绑定 hash
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (char *) hash;
    bind[3].buffer_length = strlen(hash);
    bind[3].is_null = 0;
    bind[3].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 执行更新
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        mysql_stmt_close(stmt);
        return -1;
    }

    // 清理
    mysql_stmt_close(stmt);

    // 返回成功
    return 0;
}

