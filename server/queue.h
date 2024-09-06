#ifndef QUEUE_H
#define QUEUE_H


//链式队列结点存储net_fd
typedef struct node_s
{
    int net_fd;
    struct node_s *next;
}Node;

//链式队列
//头结点指针，尾结点指针
typedef struct queue_s
{
    Node *head;
    Node *tail;
    int size;
}Queue;

//链式队列入队
//第一个参数是队列指针，第二个参数是net_fd
int enQueue(Queue *q, int net_fd);

//链式队列出队
//第一个参数是队列指针，第二个参数是存储net_fd的空间
int deQueue(Queue *q, int *net_fd);

#endif
