#include <stdlib.h>
#include "queue.h"

void inicializar_cola(Queue* queue, int quantum) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->quantum = quantum; // Asignar el quantum a la cola
}

void enqueue(Queue* queue, Process* proceso) {
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->proceso = proceso;
    new_node->siguiente = NULL;

    if (queue->rear == NULL) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->siguiente = new_node;
        queue->rear = new_node;
    }
}

Process* dequeue(Queue* queue) {
    if (is_empty(queue)) {
        return NULL;
    }
    QueueNode* temp = queue->front;
    Process* proceso = temp->proceso;
    queue->front = queue->front->siguiente;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return proceso;
}

int is_empty(Queue* queue) {
    return queue->front == NULL;
}