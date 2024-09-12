#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include "../process/process.h"
#include "../queue/queue.h"
#include "../file_manager/manager.h"

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
    inicializar_cola(&high_priority_queue, 2 * quantum);
    inicializar_cola(&low_priority_queue, quantum);

    /* Cargar procesos en la cola de alta prioridad */
    cargar_procesos(input_file, &high_priority_queue);

    /* Ejecutar el scheduler */
    ejecutar_scheduler(&high_priority_queue, &low_priority_queue);

	input_file_destroy(input_file);
}