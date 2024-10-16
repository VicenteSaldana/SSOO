#ifndef OSRMS_FILE_H
#define OSRMS_FILE_H

#include <stdio.h>
#define TOTAL_VIRTUAL_PAGES 4096
#define BITMAP_TPSO_SIZE 128 // Tamaño en bytes
#define BITMAP_TPSO_OFFSET (8 * 1024) // Offset donde comienza el bitmap de TPSO
#define PAGE_SIZE 32768
#define TPSO_OFFSET (8 * 1024 + 128) // After PCB (8KB) and bitmap of tables (128B)

// Definición de la estructura osrmsFile
typedef struct {
    int process_id;                // ID del proceso propietario del archivo
    char file_name[15];            // Nombre del archivo (14 caracteres + terminador)
    char mode;                     // Modo de apertura ('r' o 'w')
    unsigned int size;             // Tamaño del archivo en bytes
    unsigned int virtual_address;  // Dirección virtual inicial del archivo
    unsigned int current_offset;   // Offset actual para lectura/escritura
} osrmsFile;

// Prototipos de funciones
osrmsFile* os_open(int process_id, char* file_name, char mode);
int os_read_file(osrmsFile* file_desc, char* dest);
int os_write_file(osrmsFile* file_desc, char* src);
void os_close(osrmsFile* file_desc);
int allocate_second_level_page_table(int process_id, unsigned int first_level_index);
int assign_pfn_to_virtual_address(int process_id, unsigned int virtual_address, unsigned int pfn);
int allocate_tps(); // Asigna una TPSO libre y retorna su índice
void free_tps(int tps_index); // Libera una TPSO y actualiza el bitmap

#endif