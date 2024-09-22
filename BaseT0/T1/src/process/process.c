#include "process.h"
#include <stdlib.h>
#include <string.h>
#ifndef strdup
char* strdup(const char* src) {
    size_t len = strlen(src) + 1;
    char* dest = malloc(len);
    if (dest) {
        strcpy(dest, src);
    }
    return dest;
}
#endif
Process* inicializar_n_process(InputFile* input_process){
    Process* array_process =  calloc(input_process->len, sizeof(Process));
    for (int i = 0; i < input_process->len; ++i)
    {
        array_process[i].nombre = strdup(input_process->lines[i][0]);
        array_process[i].pid = atoi(input_process->lines[i][1]);
        array_process[i].estado = READY;
        array_process[i].t_entrada = atoi(input_process->lines[i][2]);
        array_process[i].burst_time = atoi(input_process->lines[i][3]);
        array_process[i].num_bursts = atoi(input_process->lines[i][4]);
        array_process[i].io_wait = atoi(input_process->lines[i][5]);
        array_process[i].deadline = atoi(input_process->lines[i][6]);
        array_process[i].num_interruptions = 0;
        array_process[i].turnaround_time = 0;
        array_process[i].response_time = 0;
        array_process[i].waiting_time = 0;
        array_process[i].waiting_time = 0;
        array_process[i].time_exceeded_deadline = 0;
        array_process[i].t_primer_uso_cpu = 0;
        array_process[i].t_lcpu = 0;
        array_process[i].t_icpu = 0;
        array_process[i].quantum = 0;
        array_process[i].tipo_cola = 0;
        array_process[i].burst_time_left = atoi(input_process->lines[i][3]);
    }
    return array_process;
}