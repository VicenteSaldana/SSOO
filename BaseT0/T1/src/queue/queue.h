#pragma once
#include "process.h"

typedef struct QueueNode {
    Process* proceso;
    struct QueueNode* siguiente;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int quantum;
} Queue;

void enqueue(Queue* queue, Process* proceso);
Process* dequeue(Queue* queue);
int is_empty(Queue* queue);

void inicializar_cola(Queue* queue, int quantum);