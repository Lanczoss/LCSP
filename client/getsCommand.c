#include "header.h"
#include <stdint.h>
#define HASH_SEED 100

// 第一个参数：路径名
// 第二个参数：文件名
// 返回值：true表示有，false表示么有
bool isExistFile(char *dir_path,char *file_name){
    DIR *pdir = opendir(dir_path);
    ERROR_CHECK(pdir,NULL,"opendir");
    struct dirent *pfile;
    while((pfile = readdir(pdir)) != NULL){
        if (strcmp(pfile->d_name,file_name) == 0 && pfile->d_type == DT_REG){
            return true;
        }
    }
    closedir(pdir);
    return false;
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

// 通过mmap接受文件内容
// 第一个参数：socket_fd与服务端通信的文件对象
// 第二个参数：file_fd文件描述符
// 第三个参数：block_num页块个数，一个页块为4096
// 第四个参数：file_size需要写入的文件的大小
// 返回值：0为成功，-1为失败
int mmapRecvFile(int socket_fd, int file_fd, int block_num, int file_size){
        // 计算还需要接受多少次文件映射的次数
        int mmap_times = (file_size - 4096 * block_num) / 4096;

        // 进行mmap的映射续传
        for (int i = 0; i < mmap_times; i++){
            // 文件映射
            int ret = ftruncate(file_fd,(i + 1 + block_num) * 4096);
            ERROR_CHECK(ret,-1,"ftruncate");
            char *mmap_file = (char *)mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, file_fd, (i + block_num) * 4096);
            ERROR_CHECK(mmap_file,NULL,"mmap");

            // 写入磁盘文件
            ret = recv(socket_fd,mmap_file,4096,MSG_WAITALL);
            ERROR_CHECK(ret,0,"recv");

            munmap(mmap_file,4096);
        }
        puts("81");
        // 读取剩余文件大小
        if ((file_size - 4096 * block_num) % 4096 != 0){
            // 文件截断扩容
            int ret = ftruncate(file_fd,file_size);
            ERROR_CHECK(ret,-1,"ftruncate");
            // 文件映射
            char *mmap_file = 
                (char *) mmap(NULL,file_size - (mmap_times + block_num) * 4096, PROT_WRITE, MAP_SHARED, file_fd, 
                             (mmap_times + block_num) * 4096);
            ERROR_CHECK(mmap_file,NULL,"mmap");
            // 写入磁盘文件
            ret = recv(socket_fd,mmap_file,file_size - (mmap_times + block_num) * 4096,MSG_WAITALL);
            ERROR_CHECK(ret,0,"recv");
            munmap(mmap_file,file_size - (mmap_times + block_num) * 4096);
        }
    return 0;
}

// 客户端接收文件函数getsCommnad
// 第一个参数：协议头部
// 第二个参数：网络连接socket_fd
// 返回值：0为正常，其他异常
int getsCommand(train_t t, int socket_fd){
    // 检错返回值
    int ret;

    // 等待接收服务端进行参数检错

    ret = recv(socket_fd,&t,sizeof(t),MSG_WAITALL);
    ERROR_CHECK(ret,-1,"recv");

    // 检查参数个数是否异常
    if (t.error_flag == ABNORMAL){
        printf("输入参数异常\n");
        return -1;
    }

    // 等待服务端服务端检查当前服务端是否有该文件
    ret = recv(socket_fd,&t,sizeof(t),MSG_WAITALL);
    ERROR_CHECK(ret,0,"recv");
    if (t.error_flag == ABNORMAL){
        printf("服务端不存在该文件\n");
        return -1;
    }

    // 处理客户端路径(相对于客户端自己主机的路径)
    char parameter[1024] = {0};
    splitParameter(t,2,parameter);
    if (parameter[strlen(parameter) - 1] == '\n'){
        parameter[strlen(parameter) - 1] = 0;
    }

    // 检查客户端是否存在该路径
    struct stat st;
    int file_fd; 
    ret = stat(parameter,&st);
    if (ret == -1 || S_ISDIR(st.st_mode) == 0){
        t.error_flag = ABNORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
        printf("客户端输入的路径不存在\n");
        return -1;
    }
    // 客户端端路径一定是一个文件夹
    else {
        t.error_flag = NORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 取出文件名
    char file_name[1024] = {0};
    splitParameter(t,1,file_name);

    // 客服端需要判断是否有同名文件
    bool flag = isExistFile(parameter,file_name);

    // 同名文件  NORMAL表示有同名文件， ABNORMAL表示没有同名文件
    if (flag == true){
        t.error_flag = NORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }
    // 无同名文件
    else {
        t.error_flag = ABNORMAL;
        ret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(ret,-1,"send");
    }

    // 拼接文件路径
    if (parameter[strlen(parameter) - 1] == '/'){
        strcat(parameter,file_name);
    }
    strcat(parameter,"/");
    strcat(parameter,file_name);
    printf("open dir: %s\n",parameter);

    // 创建文件对象
    file_fd = open(parameter,O_RDWR | O_CREAT ,0666);
    ERROR_CHECK(file_fd,-1,"open");

    // 读取服务端文件大小
    int file_size;
    ret = recv(socket_fd,&file_size,sizeof(file_size),MSG_WAITALL);
    ERROR_CHECK(ret,0,"recv");
    
    // 读取服务端文件的hash值
    char hash[50];
    ret = recv(socket_fd,hash,sizeof(hash),MSG_WAITALL);
    ERROR_CHECK(ret,0,"recv");

    // 若服务端文件大小大于4096，需要接受前4096个字节的hash值
    uint32_t stmp_hash_val;
    if (file_size > 4096){
        ret = recv(socket_fd,&stmp_hash_val,sizeof(stmp_hash_val),MSG_WAITALL);
        ERROR_CHECK(ret,0,"recv");
    }

    // 无同名文件
    if (flag == false){
        // mmap读取
        mmapRecvFile(socket_fd, file_fd, 0, file_size);
        printf("文件接受完毕\n");
        return 0;
    }
    // 有同名文件
    else if (flag == true){
        // 设置标志位
        // 设置一个int flag标志位
        // 1表示要无需发送
        // 2表示需要续传
        // 3表示需要从头发送
        int send_flag;

        // 获取当前文件的字节数
        struct stat st;
        fstat(file_fd,&st);
        int local_file_size = st.st_size;

        // 计算文件的hash值
        char local_hash[100] = {0};
        char cmd[100] = {0};
        
        // 拼接文件名与指令
        strcat(cmd,"sha1sum ");
        strcat(cmd,parameter);
        
        // 获取hash值
        FILE *pipe = popen(cmd,"r");
        ERROR_CHECK(pipe,NULL,"pipe");
        fread(local_hash,sizeof(char),sizeof(local_hash),pipe);
        
        // 关闭管道
        pclose(pipe);

        // 对hash值进行处理
        for (size_t i = 0;i < strlen(local_hash);i++){
            if (local_hash[i] == ' '){
                local_hash[i] = 0;
            }
        }
        
        // 判断是否为同一个文件
        if (strcmp(local_hash,hash) == 0){
            puts("257 --> flag = 1");
            send_flag = 1;
            ret = send(socket_fd,&send_flag,sizeof(send_flag),MSG_NOSIGNAL);
            ERROR_CHECK(ret,-1,"send");
            printf("本地已有文件无需重复下载\n");
            return 0;
        }

        // 因为文件大小需要重传
        if (file_size <= 4096 || local_file_size <= 4096){
            puts("267 --> flag = 3");
            send_flag = 3;
            ret = send(socket_fd,&send_flag,sizeof(send_flag),MSG_NOSIGNAL);
            ERROR_CHECK(ret,-1,"send");
            // 清空文件内容
            ftruncate(file_fd,0);
            // mmap下载文件
            mmapRecvFile(socket_fd, file_fd, 0, file_size);
            return 0;
        }
        // 服务端文件大小大于4096，客户端文件大小大于4096
        // 需要判断续传与重传
        else {
            // 比对前4096个字节的文件大小的hash值
            uint32_t ctmp_hash_val;
            char cfile_content[4097] = {0};
            ret = read(file_fd,cfile_content,sizeof(cfile_content) - 1);
            ERROR_CHECK(ret,0,"read");
            lseek(file_fd,0,SEEK_SET);
            ctmp_hash_val = hash_func(cfile_content,strlen(cfile_content),HASH_SEED);
            // 需要续传
            if (ctmp_hash_val == stmp_hash_val){
                puts("289 --> flag = 2");
                // 发送标记位
                send_flag = 2;
                ret = send(socket_fd,&send_flag,sizeof(send_flag),MSG_NOSIGNAL);
                ERROR_CHECK(ret,-1,"send");
                
                // 为mmap能够映射，削去多余字节
                int block_num = local_file_size / 4096 - 1;
                
                // 通知服务端从哪开始发送
                int start_send = 4096 * block_num;
                ret = send(socket_fd,&start_send,sizeof(start_send),MSG_NOSIGNAL); 
                ERROR_CHECK(ret,-1,"send");

                // 拨动文件指针到文件末尾
                lseek(file_fd,0,SEEK_END);

                printf("start_send = %d\n",start_send);

                // mmap写文件
                mmapRecvFile(socket_fd,file_fd,block_num,file_size);
                printf("续传文件完毕\n");
                return 0;
            }
            // 无需续传
            else {
                // 发送标志位
                puts("314 --> flag = 3");
                send_flag = 3;
                ret = send(socket_fd,&send_flag,sizeof(send_flag),MSG_NOSIGNAL);
                ERROR_CHECK(ret,-1,"send");
                // 清空文件内容
                ftruncate(file_fd,0);
                // 接收文件
                mmapRecvFile(socket_fd, file_fd, 0, file_size);
                printf("接收文件完毕\n");
                return 0;
            }
        }
    }
    return -1;
}

