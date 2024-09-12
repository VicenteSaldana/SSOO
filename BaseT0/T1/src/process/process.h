#pragma once

typedef enum {
    RUNNING,
    READY,
    WAITING,
    FINISHED
} ProcessState;

typedef struct {
    char* nombre;
    int pid;
    ProcessState estado;
    int burst_time;
    int num_bursts;
    int io_wait;
    int deadline;
    int tiempo_restante;

    // Atributos para el output
    int num_interruptions;
    int turnaround_time;
    int response_time;
    int waiting_time;          // Tiempo total en estado WAITING o READY
    int time_exceeded_deadline;// Suma de tiempos pasados su deadline
    int t_entrada;
    int t_primer_uso_cpu;
    int t_salida;
} Process;