#include "header.h"

int main(void)
{
    //建立socket连接

    //登录注册退出界面
    printf("（1）登录 （2）注册 (3) 退出");

    //保存从服务器发来的用户实际路径
    char user_path[1024];
    while(1)
    {
        //存储标准输入的缓冲区
        char buf[1024] = {0};
        read(STDIN_FILENO, buf, 1024);
        //拆分、分析字符串
        //将键盘输入的命令拆分成
        //路径名
        //命令序号
        //控制字符数组（命令参数）
        train_t t;
        splitCommand(&t, buf);

        //发送自定义协议
        send(socket_fd, &t, sizeof(t), MSG_NOSIGNAL);

        //判断客户端是否要发送文件
        if(t.send_flag == 1)
        {
            //客户端发送文件
            sendFile(socket_fd);
        }
        else
        {
            //接收服务器自定义协议
            bzero(&t, sizeof(t));
            recv(socket_fd, &t, sizeof(t), MSG_WAITALL);
            //处理接收的消息
            analysisProtocol(t, socket_fd);
        }
    }
}
