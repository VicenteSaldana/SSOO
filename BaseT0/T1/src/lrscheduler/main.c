#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../process/process.h"
#include "../queue/queue.h"
#include "../file_manager/manager.h"

#include <unistd.h>     // sleep

void put_ele(Queue* queue, Process* proceso) {
    QueueNode* new_node = calloc(1, sizeof(QueueNode));
    new_node->proceso = proceso;
    new_node->siguiente = NULL;

    if (queue->front == NULL)
    {
        queue->front = new_node;
    }
    else
    {
        QueueNode* temp = NULL;
        temp = queue->front;
        while (1)
        {
            if (temp->siguiente == NULL)
            {
                temp->siguiente = new_node;
                break;
            }
            temp = temp->siguiente;
        }
    }
}

void ver_cola(Queue* queue){
    QueueNode* temp = NULL;
    temp = queue->front;
    if (is_empty(queue)) {
    printf("Vacio\n");
        return;
    }
    printf("ver_cola: ");
    while (1)
    {
        printf("%s ->", temp->proceso->nombre);
        if (temp->siguiente == NULL)
        {
            break;
        }
        temp = temp->siguiente;
    }
    printf("\n");
}

Process* fist_ready(Queue* high_priority_queue, Queue* low_priority_queue, unsigned int tick){
    // Recorremos la cola de alta prioridad
    QueueNode* temp = high_priority_queue->front;
    while (temp != NULL) {
        if (temp->proceso->estado == READY) {
            printf("Proceso %s en high_priority_queue encontrado en estado READY\n", temp->proceso->nombre);
            return dequeue(high_priority_queue, tick);
        }
        temp = temp->siguiente;
    }

    // Recorremos la cola de baja prioridad
    temp = low_priority_queue->front;
    while (temp != NULL) {
        if (temp->proceso->estado == READY) {
            printf("Proceso %s en low_priority_queue encontrado en estado READY\n", temp->proceso->nombre);
            return dequeue(low_priority_queue, tick);
        }
        temp = temp->siguiente;
    }

    // No se encontró ningún proceso en estado READY en ninguna de las dos colas
    printf("No se encontró ningún proceso en estado READY en ninguna cola\n");
    return NULL;
}



void actualizar_wait(Queue* queue, unsigned int tick){
    if (queue->front == NULL){ 
        return;
    }
    QueueNode* temp = NULL;
    temp = queue->front;
    while (1){
        temp->proceso->waiting_time++;
        if (temp->proceso->estado == WAITING)
        {
            unsigned int waited = tick - temp->proceso->t_lcpu;
            if (waited >= temp->proceso->io_wait)
            {
                temp->proceso->estado = READY;
                printf("proceso %s, ya no esta en WAITING, esta READY\n", temp->proceso->nombre);
            }
            else
            {
                printf("proceso %s esta en WAITING\n", temp->proceso->nombre);
            }
        }
        else
        {
            printf("proceso %s esta en READY\n", temp->proceso->nombre);
        }
        
        if (temp->siguiente == NULL)
        {
            return;
        }
        temp = temp->siguiente;
    }
}

// 0 = proceso continua | 1 = termina por rafaga | -1 sacado por quantum 
int ver_running_procese(Process* proceso_cpu, unsigned int tick, int proceso_corriendo){
    if (proceso_corriendo == 0)
    {
        return 0;
    }
    proceso_cpu->quantum--;     // decuento del quantum
    proceso_cpu->burst_time_left--;
    if ((proceso_cpu->burst_time_left) == 0)
    {
        return 1;
    }
    else if ((proceso_cpu->quantum) <= 0)       // revisar el caso cuando el quantum pasa a negatico
    {                                           // porque el proceso cedio cuando tambien se le acabo el quantum
        return -1;
    }
    else
    {
        return 0;
    }
}

void cpu_cola(int accion,Queue* high_priority_queue, Queue* low_priority_queue, Process* proceso_cpu, int* proceso_corriendo, unsigned int tick, int* proceso_terminados, int quantum){
    if (accion == 0)
    {
        return;
    }
    else if (accion == 1)
    {
        proceso_cpu->estado = WAITING;
        proceso_cpu->num_bursts--;
        proceso_cpu->burst_time_left = proceso_cpu->burst_time;
        *proceso_corriendo = 0;
        printf("proceso %s cedio la cpu\n", proceso_cpu->nombre);
        proceso_cpu->t_lcpu = tick;
        if (proceso_cpu->num_bursts == 0)
        {
            proceso_cpu->estado = FINISHED;
            proceso_cpu->turnaround_time = tick - proceso_cpu->t_entrada;
            *proceso_terminados +=1;
            printf("proceso %s, termino\n", proceso_cpu->nombre);
            return;
        }
        if (proceso_cpu->tipo_cola == 1)
        {
            //
            // put_ele(high_priority_queue, proceso_cpu); // cambiar por el correcto despues
            enqueue(high_priority_queue, proceso_cpu); // cambiar por el correcto despues
            printf("proceso %s entro en cola high\n", proceso_cpu->nombre);
        }
        else
        {
            //
            //put_ele(low_priority_queue, proceso_cpu); // cambiar por el correcto despues
            enqueue(low_priority_queue, proceso_cpu); // cambiar por el correcto despues
            printf("proceso %s entro en cola low\n", proceso_cpu->nombre);
        }
    }
    else
    {
        proceso_cpu->estado = READY;
        if (proceso_cpu->burst_time_left == 0)
        {
            proceso_cpu->num_bursts--;
            proceso_cpu->burst_time_left = proceso_cpu->burst_time;
            proceso_cpu->estado = WAITING;
        }
        *proceso_corriendo = 0;
        printf("proceso %s fue sacadao de la cpu\n", proceso_cpu->nombre);
        proceso_cpu->t_lcpu = tick;
        proceso_cpu->num_interruptions++;
        if (proceso_cpu->num_bursts == 0)
        {
            proceso_cpu->estado = FINISHED;
            printf("tick %i, t_primer_uso_cpu %i\n", tick, proceso_cpu->t_primer_uso_cpu);
            proceso_cpu->turnaround_time = tick - proceso_cpu->t_entrada;
            *proceso_terminados +=1;
            printf("proceso %s, termino\n", proceso_cpu->nombre);
            return;
        }
        proceso_cpu->quantum = quantum;
        //
        // put_ele(low_priority_queue, proceso_cpu); // cambiar por el correcto despues
        enqueue(low_priority_queue, proceso_cpu); // cambiar por el correcto despues
        printf("proceso %s entro en cola low\n", proceso_cpu->nombre);
    }
}

void nuevo_proceso_cola(Process* array_process, Queue* high_priority_queue, long unsigned int* proceso_vivos, int n_proceso, unsigned int tick, int quantum){
    for (size_t i = *proceso_vivos; i < n_proceso; i++)
    {
        if (array_process[i].t_entrada == tick)
        {
            array_process[i].quantum = 2*quantum;
            array_process[i].tipo_cola = 1;
            //
            // put_ele(high_priority_queue, &array_process[i]);
            enqueue(high_priority_queue, &array_process[i]);
            printf("proceso %s entro en cola high\n", array_process[i].nombre);
            printf("tick %i, T_i %i\n", tick, array_process[i].t_entrada);
        }
        else
        {
            *proceso_vivos = i;
            return;
        }
    }
    *proceso_vivos = n_proceso;
    return;
}

int upgrade_cola(Process* proceso, unsigned int tick){
    return ((2*proceso->deadline) < (tick - proceso->t_lcpu));
}

void mover_low_high(Queue* low_priority_queue, Queue* high_priority_queue, unsigned int tick){
    if (is_empty(low_priority_queue)) {
        return;
    }
    QueueNode* temp = low_priority_queue->front;
    QueueNode* anterior = NULL;

    while (temp != NULL) {
        if (upgrade_cola(temp->proceso, tick)) {
            Process* proceso = temp->proceso;
            if (temp == low_priority_queue->front) {
                dequeue(low_priority_queue, tick);
                temp = low_priority_queue->front;
            } else {
                anterior->siguiente = temp->siguiente;
                if (temp == low_priority_queue->rear) {
                    low_priority_queue->rear = anterior;
                }
                QueueNode* to_free = temp;
                temp = temp->siguiente;
                free(to_free);
            }
            proceso->quantum = 2 * proceso->quantum;
            enqueue(high_priority_queue, proceso);
            printf("proceso %s, pasa a la cola high\n", proceso->nombre);
            continue;
        }
        anterior = temp;
        temp = temp->siguiente;
    }
}


void caso_quantum_0(Queue* high_priority_queue, Queue* low_priority_queue, Process** proceso_cpu, int* proceso_corriendo, unsigned int tick, int quantum){
    while (1)
    {
        
        if ((*proceso_cpu)->quantum == 0)
        {
            (*proceso_cpu)->estado = READY;
            (*proceso_cpu)->quantum = quantum;
            //
            // put_ele(low_priority_queue, (*proceso_cpu)); // cambiar por el correcto despues
            enqueue(low_priority_queue, (*proceso_cpu)); // cambiar por el correcto despues
            printf("proceso %s entro en cola low\n", (*proceso_cpu)->nombre);
        }
        else
        {
            return;
        }
        printf("No se encuentra un proceso con quantum\n");
        *proceso_cpu = fist_ready(high_priority_queue, low_priority_queue, tick);
    }
}


void correr_proceso(Queue* high_priority_queue, Queue* low_priority_queue, Process** proceso_cpu, int* proceso_corriendo, unsigned int tick, int quantum){
    if (*proceso_corriendo == 0){
        *proceso_cpu = fist_ready(high_priority_queue, low_priority_queue, tick);
        if (*proceso_cpu != NULL)
        {
            if ((*proceso_cpu)->quantum == 0){
                printf("caso_quantum_0\n");
                caso_quantum_0(high_priority_queue, low_priority_queue, proceso_cpu, proceso_corriendo, tick, quantum);
            }
            
            printf("proceso sacado: %s, con estado %i, entra en la cpu\n", (*proceso_cpu)->nombre, (*proceso_cpu)->estado);
            (*proceso_cpu)->estado = RUNNING;
            if ((*proceso_cpu)->t_primer_uso_cpu == -1)
            {
                (*proceso_cpu)->t_primer_uso_cpu = tick;
                (*proceso_cpu)->response_time = tick - (*proceso_cpu)->t_entrada;
            }
            *proceso_corriendo = 1;
            (*proceso_cpu)->t_icpu = tick;
            unsigned int t_actual = tick;
            int pasado_deadline = (t_actual - (*proceso_cpu)->t_lcpu) - (*proceso_cpu)->deadline;
            printf("t_actual: %i, t_lcpu: %i, deadline: %i, proceso %s\n", t_actual, (*proceso_cpu)->t_lcpu, (*proceso_cpu)->deadline, (*proceso_cpu)->nombre);
            printf("pasado_deadline: %i\n", pasado_deadline);
            if (pasado_deadline > 0)
            {
                (*proceso_cpu)->time_exceeded_deadline += pasado_deadline;
            }
            printf("Proceso corriendo: %s\n", (*proceso_cpu)->nombre);
            return;
        }
        else
        {
            printf("No hay proceso disponible\n");
            return;
        }
    }
    printf("Proceso corriendo: %s\n", (*proceso_cpu)->nombre);
}

void ejecutar_scheduler(Queue* high_priority_queue, Queue* low_priority_queue, Process* array_process, int quantum, int n_proceso){
    unsigned int tick = 0;
    long unsigned int proceso_vivos = 0;
    int proceso_terminados = 0;
    Process* proceso_cpu = NULL;
    int proceso_corriendo = 0;
    while (1)
    {
        printf("tick: %i\n", tick);
        actualizar_wait(high_priority_queue, tick);
        actualizar_wait(low_priority_queue, tick);
        int accion = ver_running_procese(proceso_cpu, tick, proceso_corriendo);
        cpu_cola(accion, high_priority_queue, low_priority_queue, proceso_cpu, &proceso_corriendo, tick, &proceso_terminados ,quantum);
        nuevo_proceso_cola(array_process, high_priority_queue, &proceso_vivos, n_proceso, tick, quantum);
        mover_low_high(low_priority_queue, high_priority_queue, tick);
        correr_proceso(high_priority_queue, low_priority_queue, &proceso_cpu, &proceso_corriendo, tick, quantum);
        if (proceso_terminados == n_proceso)
        {
            printf("FIN del programa\n");
            return;
        }
        if (tick > 40)
        {
            return;
        }
        
        tick++;
        // sleep(1);        // descomentar si se quiere ver paso a paso
    }
}


int main(int argc, char const *argv[])
{
    /*Lectura del input*/
    char *file_name = (char *)argv[1];
    InputFile *input_file = read_file(file_name);

    /*Mostramos el archivo de input en consola*/
    printf("Nombre archivo: %s\n", file_name);
    printf("Cantidad de procesos: %d\n", input_file->len);
    printf("Procesos:\n");
    for (int i = 0; i < input_file->len; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            printf("%s ", input_file->lines[i][j]);
        }
        printf("\n");
    }
    /* Inicializar las colas del scheduler */
    int quantum = atoi(argv[3]);
    Queue high_priority_queue, low_priority_queue;
    // inicializar_cola(&high_priority_queue, 2 * quantum);
    // inicializar_cola(&low_priority_queue, quantum);
    inicializar_cola(&high_priority_queue);
    inicializar_cola(&low_priority_queue);

    // arreglo de proceso
    Process* array_process = inicializar_n_process(input_file);
    int n_procesos = input_file->len;
    printf("lista de proceso en array_process\n");
    for (size_t i = 0; i < input_file->len; i++)
    {
        printf("proceso: %s \n", array_process[i].nombre);
    }


    /* Cargar procesos en la cola de alta prioridad */
    // cargar_procesos(input_file, &high_priority_queue);

    /* Ejecutar el scheduler */
    ejecutar_scheduler(&high_priority_queue, &low_priority_queue, array_process, quantum, n_procesos);

    for (size_t i = 0; i < n_procesos; i++)
    {
        printf("-----------------------------------------------------\n");
        printf("Nombre del proceso: %s\n", array_process[i].nombre);
        printf("PID: %i\n", array_process[i].pid);
        printf("Numero de interrupciones: %i\n", array_process[i].num_interruptions);
        printf("Turnaround time: %i\n", array_process[i].turnaround_time);
        printf("Response time: %i\n", array_process[i].response_time);
        printf("Wainting time: %i\n", array_process[i].waiting_time);
        printf("Time exceeded deadline: %i\n", array_process[i].time_exceeded_deadline);
        printf("-----------------------------------------------------\n");
    }
    for (size_t i = 0; i < n_procesos; i++)
    {
        printf("-----------------------------------------------------\n");
        printf("%s,", array_process[i].nombre);
        printf("%i,", array_process[i].pid);
        printf("%i,", array_process[i].num_interruptions);
        printf("%i,", array_process[i].turnaround_time);
        printf("%i,", array_process[i].response_time);
        printf("%i,", array_process[i].waiting_time);
        printf("%i\n", array_process[i].time_exceeded_deadline);
        printf("-----------------------------------------------------\n");
    }
    // Escribir en una archivo csv las estadisticas
    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
    perror("Error al abrir el archivo de salida");
    return 1;
    }
    for (size_t i = 0; i < n_procesos; i++)
    {
        fprintf(output_file, "%s,", array_process[i].nombre);
        fprintf(output_file, "%i,", array_process[i].pid);
        fprintf(output_file, "%i,", array_process[i].num_interruptions);
        fprintf(output_file, "%i,", array_process[i].turnaround_time);
        fprintf(output_file, "%i,", array_process[i].response_time);
        fprintf(output_file, "%i,", array_process[i].waiting_time);
        fprintf(output_file, "%i\n", array_process[i].time_exceeded_deadline);
    }
    fclose(output_file);
    for (size_t i = 0; i < n_procesos; i++) {
    free(array_process[i].nombre);
    }
    free(array_process);
    input_file_destroy(input_file);

}