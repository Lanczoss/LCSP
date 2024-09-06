#include "header.h"

int main(void)
{
    //建立socket连接

    //登录注册退出界面
    printf("（1）登录 （2）注册 (3) 退出");
    
    //epoll初始化
    //添加监听

    //保存从服务器发来的用户实际路径
    char user_path[1024];
    while(1)
    {
        struct epoll_event event[10];
        //epoll
        int epoll_num = epoll_wait(epoll_fd, event, 10, -1);
        for(int i = 0; i < epoll_num; i++)
        {
            int fd = event[i].data.fd;
            if(fd == socket_fd)
            {
                train_t t;
                //接收协议，分析服务器回复的是什么命令
                recv(socket_fd, &t, sizeof(t), MSG_WAITALL);
                analysisProtocol(t, socket_fd);
                if(t.send_flag == 0)
                {
                    //服务器回复消息
                    char buf[1024] = {0};
                    recv(socket_fd, buf, sizeof(buf), MSG_WAITALL);
                }
                else
                {
                    //服务器准备发送文件
                    //客户端准备接收文件
                    recvFile(socket_fd);
                }

            }
            if(fd == STDIN_FILENO)
            {
                //存储标准输入的缓冲区
                char buf[1024] = {0};
                read(STDIN_FILENO, buf, 1024);
                //拆分字符串
                //将键盘输入的命令拆分成
                //路径名
                //命令序号
                //控制字符数组（命令参数）
                train_t t;
                splitCommand(&t, buf);
                
                //发送自定义协议
                send(socket_fd, &t, sizeof(t), MSG_NOSIGNAL);
                
                //TODO:判断是否要发送文件

            }
        }
    }

}
