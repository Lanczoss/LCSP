#include "header.h"

int main(void)
{
    //建立socket连接
    int socket_fd;
    int ret = initSocket(&socket_fd);
    ERROR_CHECK(ret, -1, "initSocket");
    //自定义协议
    train_t t;
    bzero(&t, sizeof(t));
    //客户端的用户操作界面
    //录入用户第一次操作时的自定义协议
    //并将初始用户名加入到路径名中
    ret = interface(&t, socket_fd);
    ERROR_CHECK(ret, -1, "showInterface");

    //到这里开始服务器已经接受了用户的登录
    //此时自定义协议里有路径名及路径名长度

    while(1)
    {
        //获取新一轮的路径名
        char user_path[1024] = {0};
        strncpy(user_path, t.control_msg, t.path_length);

        //打印输入框
        printf("%s >", user_path);
        fflush(stdout);

        //存储标准输入的缓冲区
        char stdin_buf[1024] = {0};
        ssize_t rret = read(STDIN_FILENO, stdin_buf, 1024);
        ERROR_CHECK(rret, -1, "read stdin");

        //将路径名 命令 参数以buf送进splitCommand
        char buf[2048] = {0};
        strcpy(buf, user_path);
        strcat(buf, " ");
        strcat(buf, stdin_buf);

        //拆分、分析字符串
        //将键盘输入的命令拆分成
        //路径名
        //命令序号
        //控制字符数组（命令参数）
        ret = splitCommand(&t, buf);
        ERROR_CHECK(ret, -1, "splitCommand");
        //这里出来的自定义协议有基本的控制信息
        //处理接收的消息
        ret = analysisProtocol(&t, socket_fd);
        ERROR_CHECK(ret, -1, "analysisProtocol");
    }
    close(socket_fd);
}
