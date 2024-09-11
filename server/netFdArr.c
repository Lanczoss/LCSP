#include "header.h"

//创建net_fd循环数组（用于超时踢出）
int createNetFdArr(int **net_fd, int length)
{
    *net_fd = (int*)calloc(length, sizeof(int));
    ERROR_CHECK(net_fd, NULL, "calloc");
    return 0;
}

//将net_fd加入数组（暂时直接扫描空白区）
//TODO:数组链表、时间插入
int addNetFd(int net_fd, int *net_fd_arr, int length)
{
    for(int i = 0; i < length; i++)
    {
        if(net_fd_arr[i] == 0)
        {
            net_fd_arr[i] = net_fd;
            return 0;
        }
    }
    return -1;
}

