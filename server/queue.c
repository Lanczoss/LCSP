#include "header.h"
#include "queue.h"


//链式队列入队
//第一个参数是队列指针，第二个参数是net_fd
int enQueue(Queue *q, int net_fd)
{
    Node *new_node = (Node*)malloc(sizeof(Node));
    ERROR_CHECK(new_node, NULL, "malloc");
    new_node->net_fd = net_fd;
    new_node->next = NULL;
    if(q->head == NULL)
    {
        //队空
        q->head = new_node;
    }
    else
    {
        q->tail->next = new_node;
    }
    q->tail = new_node;
    q->size++;
    return 0;
}

//链式队列出队
//第一个参数是队列指针，第二个参数是net_fd
int deQueue(Queue *q, int *net_fd)
{
    if(q->head == NULL)
    {
        //队空
        return -1;
    }
    *net_fd = q->head->net_fd;
    Node *tmp = q->head;
    q->head = q->head->next;
    free(tmp);
    if(q->head == q->tail)
    {
        q->tail = NULL;
    }
    q->size--;
    return 0;
}
