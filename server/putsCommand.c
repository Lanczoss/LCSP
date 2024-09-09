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
int putsCommand(train_t t, int net_fd) {
    // 获取服务端路径
    char serve_path[256] = {0};
    snprintf(serve_path, sizeof(serve_path), "%s", BASE_PATH);
    printf("Base路径: %s\n", serve_path);

    // 获取用户路径
    char user_path[256] = {0};
    if (extractParameters(t.control_msg, 1, user_path, sizeof(user_path)) == 0 && strlen(user_path) > 0) {
        strncat(serve_path, user_path, sizeof(serve_path) - strlen(serve_path) - 1);
        //printf("拼接的服务端本地路径: %s\n", serve_path);
    } else {
        //printf("无需拼接服务端本地路径: %s\n", serve_path);
    }

    // 获取文件名
    char path_temp[256] = {0};
    if (extractParameters(t.control_msg, 2, path_temp, sizeof(path_temp)) != 0) {
        fprintf(stderr, "Failed to extract file path\n");
        return -1;
    }

    char path_name[256] = {0};
    if (path_temp[0] == '/' || path_temp[0] == '.') {
        if (extractFilename(path_temp, path_name, sizeof(path_name)) != 0) {
            fprintf(stderr, "Failed to extract filename\n");
            return -1;
        }
    } else {
        strncpy(path_name, path_temp, sizeof(path_name) - 1);
    }

    // 拼接完整路径
//    strncat(serve_path, "/", sizeof(serve_path) - strlen(serve_path) - 1);
    strncat(serve_path, path_name, sizeof(serve_path) - strlen(serve_path) - 1);

    // 去掉换行符
    size_t len = strcspn(serve_path, "\n");
    serve_path[len] = '\0'; // 确保以 '\0' 结尾

    //printf("打开文件的路径名: %s\n", serve_path);

    // 新建文件
    int open_file_fd = open(serve_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (open_file_fd < 0) {
        perror("open new file");
        return -1;
    }

    // 接收文件
    char buf[1024] = {0};
    ssize_t recv_num;
    while (1) {
        recv_num = recv(net_fd, buf, sizeof(buf), MSG_DONTWAIT);

        if (recv_num < 0) {
            perror("recv failed");
            break;
        } else if (recv_num == 0) {
            //printf("客户端关闭了连接\n");
            break;
        }

        //printf("接收到 %ld 字节\n", recv_num);
        ssize_t write_file_num = write(open_file_fd, buf, recv_num);
        if (write_file_num < 0) {
            perror("write failed");
            close(open_file_fd);
            return -1;
        }
    }



    printf("服务器成功收到文件: %s\n", serve_path);

    close(open_file_fd);
    return 0;
}

