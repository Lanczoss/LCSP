#include "header.h"
#include "gongju.h"

/**
 ******************************************************************************
 * 功能: 客户端那边上传文件过来，服务端创建文件接收客户端的文件
 * 参数: t -> 一个头部协议结构体（具体请观察 header.h），用于接收控制信息
 *         net_fd -> 与客户端通信的文件描述符
 * 返回值: 正常返回 0，失败返回 -1
 ******************************************************************************
 **/
int putsCommand(train_t t, int net_fd, MYSQL *mysql) {

    //pid
    int pid;
    //文件hash值
    char hash[256] = {0};
    //文件路径
    char file_path[256] = {0};

    // 获取服务端路径 ../files
    char serve_path[1024] = {0};
    snprintf(serve_path, sizeof(serve_path), "%s", BASE_PATH);
    serve_path[strlen(serve_path) - 1] = '\0';
    printf("基础路径%s\n", serve_path);

    // 获取用户路径  /
    char user_path[256] = {0};
    if (extractParameters(t.control_msg, 1, user_path, sizeof(user_path)) == 0 && strlen(user_path) > 0) {
        strcat(serve_path, "/");
    }
    strncpy(file_path, user_path, strlen(user_path));
    printf("处理之前的文件路径%s\n", file_path);
    if(strlen(file_path)!=1){
        strcat(file_path,"/");
    }
    printf("处理之后的文件的路径%s\n",file_path);

    printf("处理之前的用户路径%s\n", user_path);
//    if (strlen(user_path) != 1) {
//        user_path[strlen(user_path)] = '\0';
//    }
    user_path[strlen(user_path)] = '\0';
    printf("处理之后的用户路径%s\n", user_path);

    //判断服务端路径是否带/     ../files/zs/
    if (serve_path[strlen(serve_path)] == '/') {
        strcat(serve_path, "/");
    }
    printf("最终文件下载路径server_path:%s\n", serve_path);

    //提取结构体中间的第二个参数
    char two_arg[256] = {0};
    if (extractParameters(t.control_msg, 2, two_arg, sizeof(two_arg)) != 0) {
        fprintf(stderr, "提取第二个参数失败！\n");
        return -1;
    }

    // 在第二个参数中获取文件名    a.txt
    char file_name[256] = {0};
    //如果第二个参数第一个字符为/则提取，否则文件名就等于第二个参数
    if (two_arg[0] == '/' || two_arg[0] == '.') {
        int ret = extractFilename(two_arg, file_name, sizeof(file_name));
        if (ret == -1) {
            fprintf(stderr, "提取文件名失败!\n");
            return -1;
        }
    } else {
        int ret = extractFilename(two_arg, file_name, sizeof(file_name));
        if (ret == -1) {
            strncpy(file_name, two_arg, sizeof(two_arg) - 1);
        }
    }
    file_name[strlen(file_name)] = '\0';
    strncat(file_path, file_name, strlen(file_name));
    file_path[strlen(file_path)] = '\0';
    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0'; // 将换行符替换为字符串终止符
    }
    printf("最后插入数据库的文件路径:#%s#\n", file_path);

    //接收客户端文件的hash值
    int recv_hash = recv(net_fd, hash, sizeof(hash), MSG_WAITALL);
    if (recv_hash == -1) {
        printf("读取hash失败!\n");
        return -1;
    }
    hash[strlen(hash)] = '\0';

    //判断数据库是否存在该文件名，以及对比hash值是否一样
    int judge_file_exist = judgeFileExist(mysql, file_name, hash);
    if (judge_file_exist != 0) {
        //接收结构体（获取文件的总大小）
        train_t train;
        int ret = recv(net_fd, &train, sizeof(train), MSG_WAITALL);
        ERROR_CHECK(ret, -1, "recv train");

        // 拼接完整路径
        strncat(serve_path, hash, sizeof(serve_path) - strlen(serve_path) - 1);

        // 去掉换行符
        size_t len = strcspn(serve_path, "\n");
        serve_path[len] = '\0'; // 确保以 '\0' 结尾

        printf("open file:%s\n", serve_path);
        int open_file_fd = open(serve_path, O_WRONLY | O_CREAT, 0666);
        printf("open_file_fd:%d\n", open_file_fd);
        if (open_file_fd == -1) {
            printf("文件打开失败！\n");
            return -1;
        }

        //获取文件偏移量（获取现在文件的大小）
        struct stat s;
        fstat(open_file_fd, &s);

        if (train.file_length > s.st_size) {


            //获得偏移量，发送给客户端
            long offset = s.st_size;
            send(net_fd, &offset, sizeof(offset), MSG_NOSIGNAL);

            lseek(open_file_fd, offset, SEEK_SET);

            //接收文件
            char buf[1024] = {0};
            ssize_t recv_num;
            ssize_t count = 0;
            while (1) {
                recv_num = recv(net_fd, buf, sizeof(buf), MSG_DONTWAIT);
                //printf("所接收到的内容为%s\n", buf);
                if (recv_num < 0 && train.file_length - offset == count) {
                    perror("recv failed");
                    break;
                } else if (recv_num == 0) {
                    printf("客户端关闭了连接\n");
                    break;
                }

                write(open_file_fd, buf, recv_num);


                if (recv_num != -1) {
                    count += recv_num;
                }
            }
        }
        //计算hash值
        //___________________________________________________
        char sql_hash[256] = {0};
        getFileHash(serve_path, sql_hash);
        sql_hash[strlen(sql_hash)] = '\0';

        if (strcmp(sql_hash, hash) == 0) {
            //查询pid
            printf("查询pid的路径%s\n", user_path);
            queryPid(mysql, user_path, t, &pid);

            //将数据插入数据库
            uploadDatabase(mysql, file_name, t.uid, pid, file_path, hash);
            
            printf("服务器成功收到文件: %s\n", serve_path);
        }

        close(open_file_fd);
        //___________________________________________________


    } else {
        //接收结构体（获取文件的总大小）
        train_t train;
        int ret = recv(net_fd, &train, sizeof(train), MSG_WAITALL);
        ERROR_CHECK(ret, -1, "recv train");

        //查询文件是否删除，删除则修改标记为，如果为查找到则插入新记录
        int delete_flag;
        int querydeletemark = queryDeleteMark(mysql, file_name, t, file_path, hash, &delete_flag);
        if (querydeletemark == 0) {
            if (delete_flag == 0) {
                printf("文件已经存在！\n");
            } else {
                //修改标记位
                modifyDeleteMark(mysql, file_name, t, file_path, hash);
                printf("文件上传成功！\n");
            }
        } else {
            //查询pid
            printf("查询pid的路径%s\n", user_path);
            queryPid(mysql, user_path, t, &pid);

            //将数据插入数据库
            uploadDatabase(mysql, file_name, t.uid, pid, file_path, hash);
            printf("文件上传成功！\n");
        }

        long offset = -1;
        send(net_fd, &offset, sizeof(offset), MSG_NOSIGNAL);
    }

    return 0;
}
