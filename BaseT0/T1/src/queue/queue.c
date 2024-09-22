#include <stdlib.h>
#include "queue.h"

// void inicializar_cola(Queue* queue, int quantum) {
void inicializar_cola(Queue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    // queue->quantum = quantum; // Asignar el quantum a la cola
}

void enqueue(Queue* queue, Process* proceso) {
    QueueNode* new_node = calloc(1, sizeof(QueueNode));
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

int is_empty(Queue* queue) {
    return queue->front == NULL;
}

Process* dequeue(Queue* queue, unsigned int tick) {
    if (is_empty(queue)) {
        return NULL;
    }
    QueueNode* current = queue->front;
    QueueNode* prev = NULL;
    QueueNode* target_prev = NULL;
    QueueNode* target_node = NULL;
    int found = 0;

    int max_priority_value = 0;
    // Recorrer la lista para encontrar el proceso de mayor prioridad en estado READY
    while (current != NULL) {
        if (current->proceso->estado == READY) {
            int priority_value = (tick - current->proceso->t_lcpu) - current->proceso->deadline;

            // Determinar si el nodo actual tiene mayor prioridad
            if (!found || priority_value > max_priority_value || 
                (priority_value == max_priority_value && current->proceso->pid < target_node->proceso->pid)) {
                max_priority_value = priority_value;
                target_prev = prev;
                target_node = current;
                found = 1;
            }
        }

        // Avanzar al siguiente nodo
        prev = current;
        current = current->siguiente;
    }

    if (!found) {
        // No se encontró un proceso en estado READY
        return NULL;
    }

    // Remover el proceso seleccionado de la cola
    if (target_prev == NULL) {
        // Si el proceso de mayor prioridad es el primer nodo
        queue->front = target_node->siguiente;
    } else {
        target_prev->siguiente = target_node->siguiente; // Ajustar el puntero del nodo anterior al siguiente
    }

    // Si el nodo eliminado era el último, ajustar el puntero 'rear'
    if (queue->rear == target_node) {
        queue->rear = target_prev;
    }

    // Obtener el proceso del nodo eliminado
    Process* proceso_prioritario = target_node->proceso;
    free(target_node);  // Liberar memoria del nodo eliminado

    return proceso_prioritario; // Devolver el proceso de mayor prioridad
}