#include "header.h"



int removeLineBreak(char *real_path){
    // 找到换行符的位置
    size_t len = strcspn(real_path, "\n");

    // 如果找到了换行符，len 小于路径名的长度
    if (len < strlen(real_path)) {
        real_path[len] = '\0'; // 将换行符替换为字符串终止符
    }

    return 0;
}


int pwdCurrent(train_t t, int net_fd, MYSQL *mysql){
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[1];
    int id = getFileId(t, mysql); // 参数通过getfileid获得文件的索引id
    
    printf("id:%d\n", id);
    // 对id做一下错误判断
    // id如果为-1，说明表中没有这条数据
    // 那就更别提以该id为pid的文件或文件夹了
    if(id == -1){
        // 错误标志为设为 2
        // 说明表中无该路径的记录
        t.error_flag = 2;

        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);

        return -1;
    }

    // 多send一次
    send(net_fd, &t, sizeof(t), MSG_NOSIGNAL);


    char res[256] = { 0 }; // 结果集
    bool isNull;
    const char *query = "select file_name from files where pid = ?";


    // 设置字符编码格式为utf8mb4
    mysql_set_character_set(mysql, "utf8mb4");


    // 初始化预处理语句
    stmt = mysql_stmt_init(mysql);

    // 准备SQL查询
    mysql_stmt_prepare(stmt, query, strlen(query));

    // 绑定参数
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &id;
    bind[0].is_null = 0;
    bind[0].length = 0;
    mysql_stmt_bind_param(stmt, bind);

    // 执行查询
    mysql_stmt_execute(stmt);


    //存储结果集
    mysql_stmt_store_result(stmt);

    // 绑定结果
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = res;
    result[0].buffer_length = sizeof(res);
    result[0].is_null = &isNull; 

    mysql_stmt_bind_result(stmt,result);


    // 获取结果集的行数
    int row_count;
    row_count = mysql_stmt_num_rows(stmt);

    if(row_count == 0){

        printf("未查询到任何数据\n");
        // ls 0参时未查到，说明该目录下无任何文件或文件夹
        // 错误标志设为3
        t.error_flag = 3;
        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);
        // 清理资源
        mysql_stmt_close(stmt);
        return 0;

    }

    // 发送信息,查到数据
    send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);

    // 取回结果
    char buffer[1024] = { 0 };
    int len = 0;
    int blank_space = 0;
    while(mysql_stmt_fetch(stmt) == 0){

        printf("#%ld#", strlen(res));
        res[strlen(res)] = '\0';
        printf("file_name:%s\n",res);
        strncat(buffer, res, strlen(res));
        len += strlen(res);
        buffer[len + blank_space] = 32;
        blank_space++;
    }

    printf("%s\n", buffer);
    //TODO 发送内容到客户端

    int file_size = strlen(buffer);
    // 先发文件大小，再发内容
    send(net_fd, &file_size, sizeof(int), MSG_NOSIGNAL);

    send(net_fd, buffer, file_size, MSG_NOSIGNAL);

    // 清理资源
    mysql_stmt_close(stmt);
    return 0;
}

int lsCommand(train_t t, int net_fd, MYSQL *mysql){

    // 参数大于1，修改错误标志返回给客户端
    if(t.parameter_num > 1){
        // 错误标志设为1
        t.error_flag = 1;
        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);
        return 0;
    }

    // ls 直接打印用户当前所在路径下的内容
    if(t.parameter_num == 0){
        // ls后无参ls一定查表成功
        pwdCurrent(t, net_fd, mysql);
        return 0;

    }
    // ls后有一个参数
    char path[256] = { 0 };
    char para[256] = { 0 };

    // 获得客户端目前路径
    strncpy(path, t.control_msg, t.path_length);
    // 获得第一个参数
    splitParameter(t, 1, para); 

    // 如果参数不以斜杠开头，添加一个斜杠
    if(para[0] != '/'){
        size_t len = strlen(para);
        memmove(para + 1, para, len + 1);
        para[0] = '/';
    }
    
    // 去掉para后的换行符
    removeLineBreak(para);

    printf("当前path#%s#\n",path);
    printf("当前para#%s#\n",para);
    // 比较客户端目前路径和参数
    if(strcmp(path, para) == 0){
        //相等说明等价于ls
        printf("进入pwd\n");
        pwdCurrent(t, net_fd, mysql);
        printf("退出pwd");
        return 0;
    }
    
    // 参数是带斜杠的，需特殊处理当前路径不为 / 而参数只有 /
    // 非法操作
    printf("进入判断参数是否合法\n");
    if(strlen(para) == 1 && para[0] == '/'){
        printf("进入\n");
        // 错误标志设为4
        t.error_flag = 4;
        // TODO 发送错误信息

        send(net_fd, &t, sizeof(train_t), MSG_NOSIGNAL);

        return 0;
    }
    printf("未进入\n");

    
    // 检查参数是否是以当前路径开头
    if(strncmp(para, path, t.path_length) == 0){
        // 进入才查表
        train_t new_t;
        bzero(&new_t, sizeof(new_t));

        // 初始化
        new_t.command = LS;
        new_t.path_length = strlen(para);
        new_t.uid = t.uid;
        strncpy(new_t.control_msg, para, strlen(para));

        // 这时再去ls, 去获得路径的id不一定成功，需错误处理
        pwdCurrent(new_t, net_fd, mysql);
        return 0;
    }


    // 到这里说明参数不以路径开头且不相同，且以斜杠开头
    // 直接将参数拼接到路径里，再去查表
    // 未查到说明拼接目录下无内容，或者当前目录下没有该目录或者文件

    train_t new_t;
    bzero(&t, sizeof(t));

    // 初始化
    new_t.command = LS;
    // 将参数拼接到路径后
    strncat(path, para, strlen(para));
    new_t.path_length = strlen(path);
    new_t.uid = t.uid;
    strncpy(new_t.control_msg, path, strlen(path));

    // 同样去ls
    pwdCurrent(new_t, net_fd, mysql);

    return 0;
}

