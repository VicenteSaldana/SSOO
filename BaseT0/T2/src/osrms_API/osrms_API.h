#pragma once
#include "../osrms_File/Osrms_File.h"
#include <stdio.h>

extern FILE* MEMORIA; 

#define PROCESO 32
#define TAMANO_ENTRADA 256
#define INFO_PROCESO 13
#define TABLA_ARCHIVOS 115
#define ARCHIVOS 23
#define CANTIDAD_ARCHIVOS 5
#define FRAME_BITMAP_M 139392
#define LECTURA_FRAME 32
#define COLUMA_FRAME 256
#define BITMAP_T_PAGINAS_M 8192
#define LECTURA_BITMAP 4
#define COLUMA_BITMAP 32
#define TABLA_PAGINA_PO 128
#define ESPACIO_TPSO 8320
#define TAMANO_TABLA_TPSO 128

void os_mount(char* memory_path);

void os_ls_processes();

int os_exists(int process_id, char* file_name);

void os_ls_files(int process_id);

void os_frame_bitmap();

void os_tp_bitmap();

void os_start_process(int process_id, char* process_name);

void os_finish_process(int process_id);