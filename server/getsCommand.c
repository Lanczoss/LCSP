#include "header.h"
#include <stdint.h>
#define HASH_SEED 100

// 判断文件是否存在
// 第一个参数：sql对象
// 第二个参数：net_fd网络套接字
// 第三个参数：file_name文件名
// 返回值：true表示存在，false表示不存在
bool isExistReg(MYSQL *mysql, train_t t, char *file_name, char *hash){
    char select_statement[500];
    char local_path[100] = {0};
    splitParameter(t,0,local_path);
    
    sprintf(select_statement,
    "select hash from files where pid = (select id from files where uid = %d and file_path = '%s' and delete_flag = 0 and file_type = 1) and file_name = '%s' and file_type = 2" 
    ,t.uid,local_path,file_name);
    
    printf("st : %s\n",select_statement);
    
    int ret = mysql_query(mysql,select_statement);

    if (ret != 0){
        printf("%s",mysql_error(mysql));
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL){
        puts("27");
        mysql_free_result(res);
        return false;
    }
    else{
        if (mysql_num_rows(res) == 0){
            mysql_free_result(res);
            return false;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        strcpy(hash,row[0]);
        mysql_free_result(res);
        return true;
    }
}

// 根据文件名找到找到文件大小
int findFileSize(char *file_name){
    char file_path[200] = {0};
    strcat(file_path,BASE_PATH);
    if (file_path[strlen(file_path) - 1] != '/'){
        file_path[strlen(file_path)] = '/';
    }
    strcat(file_path,file_name);
    struct stat st;
    int ret = stat(file_path,&st);
    ERROR_CHECK(ret,-1,"stat");
    return st.st_size;
}

// murmur哈希函数
// 第一个参数：字符串
// 第二个参数：字符串的长度
// 第三个参数：种子值
// 返回值：哈希值
static uint32_t hash_func(const void* key, int len, uint32_t seed) {
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    uint32_t h = seed ^ len;
    const unsigned char* data = (const unsigned char*)key;

    while (len >= 4) {
        uint32_t k = *(uint32_t*)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
            h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

// 服务端getscommand函数实现
// 第一个参数：协议头部
// 第二个参数：网络连接net_fd
// 第三个参数：使用数据库对象mysql
// 返回值：0为正常，-1为异常

int getsCommand(train_t t, int net_fd, MYSQL *mysql){
    // 检错返回值
    int ret;

    // 检查参数个数是否合理
    // 参数个数异常处理
    if (t.parameter_num != 2){
        puts("114");
        t.error_flag = ABNORMAL;
        ret = send(net_fd, &t, sizeof(t), MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }
    // 参数个数正常处理
    else {
        t.error_flag = NORMAL;
        int ret =send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 读取参数
    char file_name[1024] = {0};
    splitParameter(t,1,file_name);
    
    // 判断服务器当前路径是否有这个文件
    char hash[50] = {0};
    bool flag = isExistReg(mysql, t, file_name, hash);
    if (flag == true){
        t.error_flag = NORMAL;
        ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }
    else{
        t.error_flag = ABNORMAL;
        ret = send(net_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        return -1;
    }

    // 等待客户端那边发送检错参数
    ret = recv(net_fd,&t,sizeof(t),MSG_WAITALL);
    ERROR_CHECK(ret,0,"recv");
    if (t.error_flag == ABNORMAL){
        return -1;
    }

    // 客户端判断当前目录下是否有同名文件  NORMAL表示有同名文件， ABNORMAL表示没有同名文件
    ret = recv(net_fd,&t,sizeof(t),MSG_WAITALL);
    ERROR_CHECK(ret,0,"recv");

    // 拼接文件路径
    char file_path[200] = {0};
    sprintf(file_path,"%s%s",BASE_PATH,hash);

    // 打开文件对象
    int file_fd = open(file_path,O_RDWR);
    ERROR_CHECK(file_fd,-1,"open");

    // 发送文件大小
    int file_size = findFileSize(hash);
    ret = send(net_fd,&file_size,sizeof(file_size),MSG_NOSIGNAL);
    ERROR_CHECK(ret,-1,"send");
    
    // 发送文件的hash值 
    ret = send(net_fd,hash,sizeof(hash),MSG_NOSIGNAL);
    ERROR_CHECK(ret,-1,"send");

    // 判断服务端文件大小是否大于4096,若大于4096则发送文件前4096个字节的hash值
    if (file_size > 4096){
        char sfile_content[4097] = {0};
        ret = read(file_fd,sfile_content,sizeof(sfile_content) - 1);
        lseek(file_fd,0,SEEK_SET);
        ERROR_CHECK(ret,-1,"read");
        uint32_t shash_val = hash_func(sfile_content,strlen(sfile_content),HASH_SEED);
        ret = send(net_fd,&shash_val,sizeof(shash_val),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");

        printf("shash_val = %x\n",shash_val);
    }

    printf("file_path = %s\n",file_path);
    printf("file_size = %d\n",file_size);
    printf("shash = %s\n",hash);
    

    // 无同名文件
    if (t.error_flag == ABNORMAL){
        puts("191");
        ret = sendfile(net_fd,file_fd,NULL,file_size);
        ERROR_CHECK(ret,-1,"sendfile");
        printf("文件发送完毕\n");
        return -1;
    }
    // 有同名文件
    else{
        // 客户端那边是否需要判断是否需要从头开始传还是续传或者不需要传
        // 设置一个int flag标志位
        // 1表示要无需发送
        // 2表示需要续传
        // 3表示需要从头发送
        int flag;
        puts("206");
        ret = recv(net_fd,&flag,sizeof(flag),MSG_WAITALL);
        ERROR_CHECK(ret,0,"recv");
        puts("209");

        // 判断发送状态
        if (flag == 1){
            puts("flag = 1");
            return 0;
        }
        else if (flag == 2){
            puts("flag = 2");
            // 接收客户端文件大小
            int start_send;
            ret = recv(net_fd,&start_send,sizeof(start_send),MSG_WAITALL);
            ERROR_CHECK(ret,-1,"recv");
            int count = file_size - start_send;

            // 发送文件内容
            off_t off = start_send;
            printf("off = %ld\n",off);
            printf("conut1 = %d\n",count);
            ret = sendfile(net_fd, file_fd, &off, count);
            ERROR_CHECK(ret,-1,"sendfille");
            printf("文件发送完毕: %d \n", ret);
            return 0;
        }
        else if (flag == 3){
            puts("flag = 3");
            ret = sendfile(net_fd,file_fd,NULL,file_size);
            ERROR_CHECK(ret,-1,"sendfile");
            printf("文件发送完毕\n");
            return 0;
        }
    }
    return 0;
}
