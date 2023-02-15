#ifndef COLA_H_
#define COLA_H_

typedef struct node {
        struct node* next;
        int *client_socket;
} node_t;



int *dequeue();
void enqueue(int *client_socket);

#endif // COLA_H_