#include "header.h"
//判断配置文件config是否存在
int checkConfig(void)
{
    FILE *fp = fopen("config.ini", "r");
    ERROR_CHECK(fp, NULL, "config.ini doesn't exist.");
    fclose(fp);
    return 0;
}

char user_path[1024] = {0};
//接收SIGINT的信号处理函数
void exitFunc(int num)
{
    //打印输入框
    printf("\n\033[38;5;208m[\xF0\x9F\xA5\xB3\xE2\x98\x81  \033[38;5;118m%s\033[38;5;208m]\033[0m ", user_path);
    fflush(stdout);
}

int main(void)
{
    signal(SIGINT, exitFunc);
    LOG_INFO("Start cloud storage service client.");
    int ret = checkConfig();
    if(ret == -1)
    {
        printf("Please make sure config.ini correct.\n");
        exit(1);
    }
    //建立socket连接
    int socket_fd;
    
    //自定义协议
    train_t t;
    bzero(&t, sizeof(t));
    t.isLoginFailed = 1;
    //客户端的用户操作界面
    ret = interface(&t, &socket_fd);
    if(ret == -1)
    {
        //函数出错或者退出
        close(socket_fd);
        exit(0);
    }
    //录入用户第一次操作时的自定义协议
    //并将初始用户名加入到路径名中
    while(1)
    { 
        signal(SIGINT, exitFunc);
        //到这里开始服务器已经接受了用户的登录
        //此时自定义协议里有路径名及路径名长度
        //printf("登录成功时 t->control_msg = %s\nt->path_length = %d\n", t.control_msg, t.path_length);
        //获取新一轮的路径名
        bzero(user_path, sizeof(user_path));
        strncpy(user_path, t.control_msg, t.path_length);

        //打印输入框
        printf("\033[38;5;208m[\xF0\x9F\xA5\xB3\xE2\x98\x81  \033[38;5;118m%s\033[38;5;208m]\033[0m ", user_path);
        fflush(stdout);

        //存储标准输入的缓冲区
        char stdin_buf[1024] = {0};
        ssize_t rret = read(STDIN_FILENO, stdin_buf, 1024);
        ERROR_CHECK(rret, -1, "read stdin");
        //printf("\n");
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
        //发送命令
        rret = send(socket_fd,&t,sizeof(t),MSG_NOSIGNAL);
        ERROR_CHECK(rret, -1, "send");

        ret = analysisProtocol(&t, socket_fd);
        if(ret == -1)
        {
            close(socket_fd);
            exit(0);
        }
        printf("\n");
    }
    close(socket_fd);
}
