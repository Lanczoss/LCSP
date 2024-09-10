#include "header.h"
//判断配置文件config是否存在
int checkConfig(void)
{
    FILE *fp = fopen("config.ini", "r");
    if(fp == NULL)
    {
        LOG_PERROR("config.ini不存在，服务正在退出");
        return -1;
    }
    fclose(fp);
    return 0;
}

int main(void)
{
    int ret = checkConfig();
    if(ret == -1)
    {
        exit(1);
    }
    //建立socket连接
    int socket_fd;
    ret = initSocket(&socket_fd);
    ERROR_CHECK(ret, -1, "initSocket");
    LOG_INFO("socket建立成功");
    //自定义协议
    train_t t;
    bzero(&t, sizeof(t));
    //客户端的用户操作界面
    //录入用户第一次操作时的自定义协议
    //并将初始用户名加入到路径名中
    ret = interface(&t, socket_fd);
    if(ret == -1)
    {
        //函数出错或者退出
        close(socket_fd);
        exit(0);
    }
    //到这里开始服务器已经接受了用户的登录
    //此时自定义协议里有路径名及路径名长度
    //printf("登录成功时 t->control_msg = %s\nt->path_length = %d\n", t.control_msg, t.path_length);

    while(1)
    {
        //获取新一轮的路径名
        char user_path[1024] = {0};
        strncpy(user_path, t.control_msg, t.path_length);

        //打印输入框
        printf("Cloud %s> ", user_path);
        fflush(stdout);

        //存储标准输入的缓冲区
        char stdin_buf[1024] = {0};
        ssize_t rret = read(STDIN_FILENO, stdin_buf, 1024);
        ERROR_CHECK(rret, -1, "read stdin");
        LOG_INFO("检测到键盘输入");

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

        LOG_INFO("拆分结束，发送");
        
        //发送命令
        rret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(rret, -1, "send");

        ret = analysisProtocol(&t, socket_fd);
        if(ret == -1)
        {
            close(socket_fd);
            exit(0);
        }
    }
    close(socket_fd);
}
