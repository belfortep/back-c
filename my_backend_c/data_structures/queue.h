#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct node {
        struct node* next;
        int *client_socket;
} node_t;



int *dequeue();
void enqueue(int *client_socket);

#endif // QUEUE_H_