#pragma once
#include "../file_manager/manager.h"

typedef enum {
    RUNNING,
    READY,
    WAITING,
    FINISHED
} ProcessState;

typedef struct {
    char* nombre;
    int pid;
    unsigned int t_entrada;
    ProcessState estado;
    unsigned int burst_time;
    unsigned int num_bursts;
    unsigned int io_wait;
    unsigned int deadline;
    // int tiempo_restante;

    // Atributos para el output
    unsigned int num_interruptions;
    unsigned int turnaround_time;
    unsigned int response_time;
    unsigned int waiting_time;          // Tiempo total en estado WAITING o READY
    unsigned int time_exceeded_deadline;// Suma de tiempos pasados su deadline
    unsigned int t_primer_uso_cpu;
    unsigned int t_lcpu;         // tiempo cuando salio por ultima vez la CPU
    unsigned int t_icpu;             // tiempo cuando entro por ultima vez la CPU
    int quantum;
    int tipo_cola;      // 0 no definido, 1 cola high, 2 cola low
    unsigned int burst_time_left;
} Process;

Process* inicializar_n_process(InputFile* input_process);