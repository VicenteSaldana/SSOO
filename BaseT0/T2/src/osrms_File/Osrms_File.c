// osrms_file.c
#include "Osrms_File.h"
#include "../osrms_API/osrms_API.h" // Asegúrate de ajustar la ruta según tu estructura de carpetas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Prototipos de funciones auxiliares
unsigned int translate_address(int process_id, unsigned int virtual_address);
int allocate_frame();
unsigned int assign_virtual_address(int process_id, unsigned int file_size);
int update_file_entry(int process_id, char* file_name, unsigned int new_size, unsigned int new_virtual_address);

// Implementación de os_open
osrmsFile* os_open(int process_id, char* file_name, char mode) {
    if (mode != 'r' && mode != 'w') {
        printf("Modo de apertura inválido\n");
        return NULL;
    }
    // Busca el proceso en la tabla de PCBs
    unsigned char proceso[INFO_PROCESO];
    for (size_t i = 0; i < PROCESO; i++) {
        fseek(MEMORIA, TAMANO_ENTRADA * i, SEEK_SET);
        size_t read_bytes = fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (read_bytes != INFO_PROCESO) {
            continue; // Error al leer, continuar
        }
        if (proceso[0] == 0) { // Proceso inactivo
            continue;
        }
        if (proceso[1] != process_id) { // No es el proceso buscado
            continue;
        }

        // Posición inicial de la tabla de archivos
        // long int tabla_archivos_pos = ftell(MEMORIA);
        if (mode == 'r') {
            printf("Modo de apertura: Lectura\n");
            unsigned char entrada_archivo[ARCHIVOS];
            for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++) {
                size_t read_archivo = fread(entrada_archivo, sizeof(unsigned char), ARCHIVOS, MEMORIA);
                if (read_archivo != ARCHIVOS) {
                    continue; // Error al leer, continuar
                }
                if (entrada_archivo[0] == 0) { // Entrada no válida
                    continue;
                }
                char nombre_archivo[15];
                strncpy(nombre_archivo, (char*)&entrada_archivo[1], 14);
                nombre_archivo[14] = '\0';
                if (strcmp(nombre_archivo, file_name) == 0) {
                    // Archivo encontrado, crear osrmsFile
                    osrmsFile* file = (osrmsFile*)malloc(sizeof(osrmsFile));
                    if (!file) {
                        perror("Error al asignar memoria para osrmsFile");
                        return NULL;
                    }
                    file->process_id = process_id;
                    strncpy(file->file_name, nombre_archivo, 15);
                    file->mode = mode;

                    // Obtener tamaño del archivo (bytes 15-18, little endian)
                    file->size = entrada_archivo[15] |
                                (entrada_archivo[16] << 8) |
                                (entrada_archivo[17] << 16) |
                                (entrada_archivo[18] << 24);

                    // Obtener dirección virtual (bytes 19-22, little endian)
                    file->virtual_address = entrada_archivo[19] |
                                            (entrada_archivo[20] << 8) |
                                            (entrada_archivo[21] << 16) |
                                            (entrada_archivo[22] << 24);

                    // Inicializar offset
                    file->current_offset = 0;
                    printf("Archivo encontrado\n");
                    return file;
                }
            }
            printf("Archivo no encontrado\n");
            // Archivo no encontrado
            return NULL;
        }
        if (mode == 'w') {
            printf("Modo de apertura: Escritura\n");
            unsigned char entrada_archivo[ARCHIVOS];
            long int entrada_libre_pos = -1;
            for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++) {
                long int entrada_pos = ftell(MEMORIA);
                size_t read_archivo = fread(entrada_archivo, sizeof(unsigned char), ARCHIVOS, MEMORIA);
                if (read_archivo != ARCHIVOS) {
                    continue; // Error al leer, continuar
                }
                if (entrada_archivo[0] == 0) { // Entrada libre
                    if (entrada_libre_pos == -1) {
                        entrada_libre_pos = entrada_pos;
                    }
                    printf("Entrada libre encontrada\n");
                    continue;
                }
                char nombre_archivo[15];
                strncpy(nombre_archivo, (char*)&entrada_archivo[1], 14);
                nombre_archivo[14] = '\0';
                if (strcmp(nombre_archivo, file_name) == 0) {
                    // Archivo ya existe
                    printf("El archivo ya existe\n");
                    return NULL;
                }
            }

            if (entrada_libre_pos == -1) {
                // No hay espacio en la tabla de archivos
                printf("No hay espacio en la tabla de archivos\n");
                return NULL;
            }

            // Crear nueva entrada de archivo
            unsigned char nueva_entrada[ARCHIVOS];
            memset(nueva_entrada, 0, ARCHIVOS);
            nueva_entrada[0] = 0x01; // Entrada válida
            strncpy((char*)&nueva_entrada[1], file_name, 14);
            // Tamaño inicial 0 (bytes 15-18)
            nueva_entrada[15] = 0x00;
            nueva_entrada[16] = 0x00;
            nueva_entrada[17] = 0x00;
            nueva_entrada[18] = 0x00;
            // Dirección virtual inicial 0 (bytes 19-22)
            nueva_entrada[19] = 0x00;
            nueva_entrada[20] = 0x00;
            nueva_entrada[21] = 0x00;
            nueva_entrada[22] = 0x00;

            // Escribir la nueva entrada en la tabla de archivos
            fseek(MEMORIA, entrada_libre_pos, SEEK_SET);
            size_t write_result = fwrite(nueva_entrada, sizeof(unsigned char), ARCHIVOS, MEMORIA);
            if (write_result != ARCHIVOS) {
                perror("Error al escribir la nueva entrada de archivo");
                return NULL;
            }
            fflush(MEMORIA); // Asegura que los datos se escriban en el archivo

            // Crear osrmsFile
            osrmsFile* file = (osrmsFile*)malloc(sizeof(osrmsFile));
            if (!file) {
                perror("Error al asignar memoria para osrmsFile");
                return NULL;
            }
            file->process_id = process_id;
            strncpy(file->file_name, file_name, 15);
            file->mode = mode;
            file->size = 0;
            file->virtual_address = 0; // Se asignará al escribir
            file->current_offset = 0;

            return file;
        }
        printf("Modo de apertura inválido\n");
        return NULL;
    }
    printf("Proceso no encontrado\n");
    return NULL;
}

    // Implementación de os_close
    void os_close(osrmsFile* file_desc) {
        if (file_desc) {
            free(file_desc);
        }
    }

    // Implementación de os_read_file
    int os_read_file(osrmsFile* file_desc, char* dest) {
        if (!file_desc || file_desc->mode != 'r') {
            printf("Error en el archivo o modo de apertura\n");
            printf("file_desc: %p\n", file_desc);
            printf("file_desc->mode: %c\n", file_desc->mode);
            return -1;
        }

        // Abrir el archivo de destino para escritura
        FILE* dest_file = fopen(dest, "wb");
        if (!dest_file) {
            perror("Error al abrir el archivo de destino");
            return -1;
        }

        unsigned int bytes_leidos = 0;
        unsigned int total_leidos = 0;

        while (bytes_leidos < file_desc->size) {
            // Calcular la dirección virtual actual
            unsigned int virtual_address = file_desc->virtual_address + file_desc->current_offset;

            // Traducir dirección virtual a física
            unsigned int physical_address = translate_address(file_desc->process_id, virtual_address);
            if (physical_address == (unsigned int)-1) {
                // Error en la traducción de dirección
                fclose(dest_file);
                return total_leidos;
            }

            // Leer desde la memoria física
            fseek(MEMORIA, physical_address, SEEK_SET);
            unsigned char buffer[32 * 1024]; // 32 KB por frame
            size_t bytes_a_leer = (file_desc->size - bytes_leidos) < (32 * 1024) ? (file_desc->size - bytes_leidos) : (32 * 1024);
            size_t leidos = fread(buffer, sizeof(unsigned char), bytes_a_leer, MEMORIA);
            if (leidos == 0) {
                break; // Fin del archivo o error
            }

            // Escribir en el archivo de destino
            size_t write_result = fwrite(buffer, sizeof(unsigned char), leidos, dest_file);
            if (write_result != leidos) {
                perror("Error al escribir en el archivo de destino");
                fclose(dest_file);
                return total_leidos;
            }
            total_leidos += leidos;
            bytes_leidos += leidos;
            file_desc->current_offset += leidos;
        }

        fclose(dest_file);
        return total_leidos;
    }

    // Implementación de os_write_file
 int os_write_file(osrmsFile* file_desc, char* src) {
    if (!file_desc || file_desc->mode != 'w') {
        printf("Error en el archivo o modo de apertura\n");
        printf("file_desc: %p\n", file_desc);
        printf("file_desc->mode: %c\n", file_desc->mode);
        return -1;
    }

    // Abrir el archivo fuente para lectura
    FILE* src_file = fopen(src, "rb");
    if (!src_file) {
        perror("Error al abrir el archivo fuente");
        return -1;
    }

    // Obtener el tamaño del archivo fuente
    fseek(src_file, 0, SEEK_END);
    long src_size = ftell(src_file);
    fseek(src_file, 0, SEEK_SET);

    if (src_size < 0) {
        fclose(src_file);
        printf("Error al obtener el tamaño del archivo fuente\n");
        return -1;
    }
    printf("Tamaño del archivo fuente: %ld bytes\n", src_size);

    // Leer el contenido del archivo fuente
    unsigned char* buffer = (unsigned char*)malloc(src_size);
    if (!buffer) {
        fclose(src_file);
        printf("Error al asignar memoria para el buffer\n");
        return -1;
    }
    printf("Leyendo archivo fuente...\n");
    size_t leidos = fread(buffer, sizeof(unsigned char), src_size, src_file);
    printf("Bytes leídos: %zu\n", leidos);
    fclose(src_file);

    // Asignar dirección virtual si es la primera escritura
    if (file_desc->virtual_address == 0 && file_desc->size == 0) {
        unsigned int new_virtual_address = assign_virtual_address(file_desc->process_id, src_size);
        if (new_virtual_address == 0xFFFFFFFF) {
            // No hay direcciones virtuales disponibles
            free(buffer);
            printf("No hay direcciones virtuales disponibles\n");
            return -1;
        }
        file_desc->virtual_address = new_virtual_address;
        file_desc->current_offset = 0; // Iniciamos desde el offset 0 relativo al archivo

        // Actualizar la tabla de archivos en mem_format.bin con la nueva dirección virtual
        update_file_entry(file_desc->process_id, file_desc->file_name, src_size, new_virtual_address);
    }

    // Escribir en la memoria simulada
    unsigned int bytes_escritos = 0;

    while (bytes_escritos < leidos) {
        // Calcular la dirección virtual actual
        unsigned int virtual_address = file_desc->virtual_address + bytes_escritos;

        // Traducir dirección virtual a física
        unsigned int physical_address = translate_address(file_desc->process_id, virtual_address);
        if (physical_address == (unsigned int)-1) {
            // No se pudo traducir la dirección, intentar asignar una nueva página
            int frame_number = allocate_frame();
            if (frame_number == -1) {
                // No hay frames disponibles
                free(buffer);
                printf("No hay frames disponibles\n");
                return -1;
            }

            // Asignar el PFN a la dirección virtual
            assign_pfn_to_virtual_address(file_desc->process_id, virtual_address, frame_number);

            // Reintentar traducir la dirección virtual ahora que tiene un frame asignado
            physical_address = translate_address(file_desc->process_id, virtual_address);
            if (physical_address == (unsigned int)-1) {
                // Aún no se pudo traducir
                free(buffer);
                printf("Error al traducir la dirección virtual después de asignar el frame\n");
                return -1;
            }
        }

        // Calcular cuántos bytes podemos escribir en este frame
        unsigned int offset_within_page = virtual_address % PAGE_SIZE;
        unsigned int bytes_restantes_en_pagina = PAGE_SIZE - offset_within_page;
        unsigned int bytes_a_escribir = (leidos - bytes_escritos) < bytes_restantes_en_pagina ? (leidos - bytes_escritos) : bytes_restantes_en_pagina;

        // Escribir en la memoria física
        fseek(MEMORIA, physical_address, SEEK_SET);
        size_t escritos = fwrite(buffer + bytes_escritos, sizeof(unsigned char), bytes_a_escribir, MEMORIA);
        if (escritos != bytes_a_escribir) {
            perror("Error al escribir en la memoria física");
            free(buffer);
            return -1;
        }

        // Actualizar el archivo de memoria inmediatamente
        fflush(MEMORIA);

        bytes_escritos += escritos;
        file_desc->current_offset += escritos;
    }

    // Actualizar el tamaño del archivo en la tabla de archivos
    file_desc->size += bytes_escritos;

    // Actualizar la entrada del archivo en la tabla de archivos dentro de la memoria simulada
    update_file_entry(file_desc->process_id, file_desc->file_name, file_desc->size, file_desc->virtual_address);
    printf("Archivo escrito exitosamente\n");
    free(buffer);
    return bytes_escritos;
}

    // Función para traducir dirección virtual a física

    unsigned int translate_address(int process_id, unsigned int virtual_address) {
    unsigned int VPN = (virtual_address >> 15) & 0xFFF; // 12 bits
    unsigned int offset = virtual_address & 0x7FFF;    // 15 bits

    unsigned int first_6_bits = (VPN >> 6) & 0x3F;

    // Obtener la dirección de la Tabla de Páginas de Primer Orden (PPO) para el proceso
    unsigned int pcb_offset = (process_id % PROCESO) * TAMANO_ENTRADA;
    unsigned int tabla_pags_primer_orden_pos = 8 * 1024 + pcb_offset + TABLA_PAGINA_PO;

    unsigned char ppo_entry[2];
    fseek(MEMORIA, tabla_pags_primer_orden_pos + (first_6_bits * 2), SEEK_SET);
    fread(ppo_entry, sizeof(unsigned char), 2, MEMORIA);
    unsigned int sptn = ppo_entry[0] | (ppo_entry[1] << 8);

    // Verificar si la SPO está asignada, 0xFFFF indica que no está asignada
    if (sptn == 0xFFFF) {
        return (unsigned int)-1; // Retornar -1 para indicar que no hay SPO asignada
    }

    unsigned int last_6_bits = VPN & 0x3F;
    unsigned int tabla_pags_segundo_orden_pos = 8 * 1024 + 128 + 128 * 1024 + 8 * 1024;
    tabla_pags_segundo_orden_pos += sptn * 128 + last_6_bits * 2;

    unsigned char pfn_entry[2];
    fseek(MEMORIA, tabla_pags_segundo_orden_pos, SEEK_SET);
    fread(pfn_entry, sizeof(unsigned char), 2, MEMORIA);
    unsigned int pfn = pfn_entry[0] | (pfn_entry[1] << 8);

    if (pfn == 0xFFFF) {
        return (unsigned int)-1; // Indica que la entrada de PFN es inválida
    }

    unsigned int physical_address = (pfn * (32 * 1024)) + offset;
    return physical_address;
}

    // Implementación de allocate_frame
    int allocate_frame() {
        unsigned char byte;
        for (unsigned int i = 0; i < (FRAME_BITMAP_M / 8); i++) { // Asumiendo FRAME_BITMAP_M es el offset de inicio del Frame Bitmap
            fseek(MEMORIA, FRAME_BITMAP_M + i, SEEK_SET);
            size_t read_byte = fread(&byte, sizeof(unsigned char), 1, MEMORIA);
            if (read_byte != 1) {
                continue; // Error al leer, continuar
            }
            for (int bit = 0; bit < 8; bit++) {
                if (((byte >> (7 - bit)) & 0x01) == 0) { // Frame libre encontrado
                    byte |= (1 << (7 - bit)); // Marcar como ocupado
                    fseek(MEMORIA, FRAME_BITMAP_M + i, SEEK_SET);
                    size_t write_result = fwrite(&byte, sizeof(unsigned char), 1, MEMORIA);
                    if (write_result != 1) {
                        perror("Error al marcar el frame como ocupado");
                        return -1;
                    }
                    fflush(MEMORIA); // Asegura que los datos se escriban en el archivo
                    return (i * 8) + bit; // Retornar el número de frame
                }
            }
        }
        // No hay frames libres
        return -1;
    }

// Función para asignar una dirección virtual disponible
unsigned int assign_virtual_address(int process_id, unsigned int file_size) {
    printf("Asignando nueva dirección virtual para el proceso ID: %d\n", process_id);

    // Calculate number of pages needed
    unsigned int pages_needed = (file_size + PAGE_SIZE - 1) / PAGE_SIZE;

    // Iterate over TPO entries (0 to 63)
    for (unsigned int tpo_index = 0; tpo_index < 64; tpo_index++) {
        unsigned int tpo_base_virtual_address = tpo_index * 64 * PAGE_SIZE; // Base virtual address for this TPO

        // Check existing files in this TPO
        unsigned int largest_end_offset = 0;

        // Read the process's file table
        unsigned char proceso[INFO_PROCESO];
        fseek(MEMORIA, TAMANO_ENTRADA * process_id, SEEK_SET);
        fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);

        // Position in file table
        unsigned int file_table_pos = ftell(MEMORIA);

        for (size_t i = 0; i < CANTIDAD_ARCHIVOS; i++) {
            unsigned char entrada_archivo[ARCHIVOS];
            fseek(MEMORIA, file_table_pos + i * ARCHIVOS, SEEK_SET);
            fread(entrada_archivo, sizeof(unsigned char), ARCHIVOS, MEMORIA);

            if (entrada_archivo[0] == 0x00) {
                continue; // Invalid entry
            }

            // Get virtual address
            unsigned int file_virtual_address = entrada_archivo[19] |
                                                (entrada_archivo[20] << 8) |
                                                (entrada_archivo[21] << 16) |
                                                (entrada_archivo[22] << 24);

            // Get size
            unsigned int existing_file_size = entrada_archivo[15] |
                                              (entrada_archivo[16] << 8) |
                                              (entrada_archivo[17] << 16) |
                                              (entrada_archivo[18] << 24);

            // Compute TPO index of the file
            unsigned int file_tpo_index = (file_virtual_address >> 21) & 0x3F;

            if (file_tpo_index == tpo_index) {
                // File is in the same TPO
                unsigned int file_offset_within_tpo = file_virtual_address & 0x1FFFFF; // 21 bits for offset within TPO
                unsigned int file_end_offset = file_offset_within_tpo + existing_file_size;
                if (file_end_offset > largest_end_offset) {
                    largest_end_offset = file_end_offset;
                }
            }
        }

        // Now, see if there is space in this TPO
        if (largest_end_offset + file_size <= 2 * 1024 * 1024) { // 2 MB per TPO
            // There is space
            unsigned int new_virtual_address = tpo_base_virtual_address + largest_end_offset;
            printf("Asignando dirección virtual: %u en TPO %u con offset %u\n", new_virtual_address, tpo_index, largest_end_offset);
            return new_virtual_address;
        }
    }

    // No space found
    printf("No se encontraron espacios libres suficientes en la memoria virtual para el proceso ID: %d\n", process_id);
    return 0xFFFFFFFF;
}

    
    // Implementación de update_file_entry
    int update_file_entry(int process_id, char* file_name, unsigned int new_size, unsigned int new_virtual_address) {
        unsigned char proceso[INFO_PROCESO];
        unsigned char entrada_archivo[ARCHIVOS];
        for (size_t i = 0; i < PROCESO; i++) {
            fseek(MEMORIA, TAMANO_ENTRADA * i, SEEK_SET);
            size_t read_proceso = fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
            if (read_proceso != INFO_PROCESO) {
                continue; // Error al leer, continuar
            }
            if (proceso[0] == 0 || proceso[1] != process_id) {
                continue; // Proceso no activo o no coincide
            }

            // Posición inicial de la tabla de archivos
            long int tabla_archivos_pos = ftell(MEMORIA);

            for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++) {
                long int entrada_pos = tabla_archivos_pos + (j * ARCHIVOS);
                fseek(MEMORIA, entrada_pos, SEEK_SET);
                size_t read_archivo = fread(entrada_archivo, sizeof(unsigned char), ARCHIVOS, MEMORIA);
                if (read_archivo != ARCHIVOS) {
                    continue; // Error al leer, continuar
                }
                if (entrada_archivo[0] == 0) {
                    continue; // Entrada no válida
                }

                char nombre_archivo[15];
                strncpy(nombre_archivo, (char*)&entrada_archivo[1], 14);
                nombre_archivo[14] = '\0';

                if (strcmp(nombre_archivo, file_name) == 0) {
                    // Actualizar tamaño (bytes 15-18, little endian)
                    entrada_archivo[15] = new_size & 0xFF;
                    entrada_archivo[16] = (new_size >> 8) & 0xFF;
                    entrada_archivo[17] = (new_size >> 16) & 0xFF;
                    entrada_archivo[18] = (new_size >> 24) & 0xFF;

                    // Actualizar dirección virtual (bytes 19-22, little endian)
                    entrada_archivo[19] = new_virtual_address & 0xFF;
                    entrada_archivo[20] = (new_virtual_address >> 8) & 0xFF;
                    entrada_archivo[21] = (new_virtual_address >> 16) & 0xFF;
                    entrada_archivo[22] = (new_virtual_address >> 24) & 0xFF;

                    // Escribir de nuevo en la memoria
                    fseek(MEMORIA, entrada_pos, SEEK_SET);
                    size_t write_result = fwrite(entrada_archivo, sizeof(unsigned char), ARCHIVOS, MEMORIA);
                    if (write_result != ARCHIVOS) {
                        perror("Error al actualizar la entrada de archivo");
                        return -1;
                    }
                    fflush(MEMORIA);
                    return 0;
                }
            }
        }
        return -1;
    }

    // Implementación de assign_pfn_to_virtual_address
int assign_pfn_to_virtual_address(int process_id, unsigned int virtual_address, unsigned int pfn) {
    // Extract VPN
    unsigned int VPN = virtual_address / PAGE_SIZE; // 0 to 4095

    // Extract first and last 6 bits
    unsigned int first_6_bits = (VPN >> 6) & 0x3F;
    unsigned int last_6_bits = VPN & 0x3F;

    // Get the position of the PPO
    unsigned int pcb_offset = (process_id % PROCESO) * TAMANO_ENTRADA;
    unsigned int tabla_pags_primer_orden_pos = pcb_offset + INFO_PROCESO; // PPO starts after process info

    // Read the SPTN from PPO
    unsigned char ppo_entry[2];
    fseek(MEMORIA, tabla_pags_primer_orden_pos + (first_6_bits * 2), SEEK_SET);
    fread(ppo_entry, sizeof(unsigned char), 2, MEMORIA);
    unsigned int sptn = ppo_entry[0] | (ppo_entry[1] << 8);

    if (sptn == 0xFFFF) {
        // Allocate new TPSO
        sptn = allocate_second_level_page_table(process_id, first_6_bits);
        if (sptn == -1) {
            return -1;
        }
    }

    // Write the PFN into the TPSO
    unsigned int tabla_pags_segundo_orden_pos = TPSO_OFFSET + sptn * 128 + last_6_bits * 2;

    unsigned char pfn_entry[2];
    pfn_entry[0] = pfn & 0xFF;
    pfn_entry[1] = (pfn >> 8) & 0xFF;

    fseek(MEMORIA, tabla_pags_segundo_orden_pos, SEEK_SET);
    fwrite(pfn_entry, sizeof(unsigned char), 2, MEMORIA);
    fflush(MEMORIA);

    return 0;
}

    // Función para asignar una nueva tabla de segundo orden
int allocate_second_level_page_table(int process_id, unsigned int first_level_index) {
    int sptn = allocate_tps(); // Obtener una TPSO libre
    if (sptn == -1) {
        printf("No hay tablas de páginas de segundo orden disponibles.\n");
        return -1;
    }

    // Inicializar la TPSO con entradas inválidas (0xFFFF)
    unsigned char invalid_entry[2] = {0xFF, 0xFF};
    unsigned int tps_offset = BITMAP_TPSO_OFFSET + sptn * 128; // Cada TPSO tiene 128 bytes

    fseek(MEMORIA, tps_offset, SEEK_SET);
    for (int i = 0; i < 64; i++) {
        fwrite(invalid_entry, sizeof(unsigned char), 2, MEMORIA);
    }
    fflush(MEMORIA);

    // Actualizar la TPO del proceso con el nuevo SPTN
    unsigned char sptn_entry[2];
    sptn_entry[0] = sptn & 0xFF;
    sptn_entry[1] = (sptn >> 8) & 0xFF;

    unsigned int tpo_offset =  (process_id % PROCESO) * TAMANO_ENTRADA + TABLA_PAGINA_PO;
    fseek(MEMORIA, tpo_offset + (first_level_index * 2), SEEK_SET);
    fwrite(sptn_entry, sizeof(unsigned char), 2, MEMORIA);
    fflush(MEMORIA);

    return sptn;
    }


int allocate_tps() {
    unsigned char byte;
    int tps_index = -1;

    for (int i = 0; i < 1024; i++) { // Suponiendo 1024 TPSO disponibles
        int byte_index = i / 8;
        int bit_index = 7 - (i % 8);

        fseek(MEMORIA, BITMAP_TPSO_OFFSET + byte_index, SEEK_SET);
        size_t read_byte = fread(&byte, sizeof(unsigned char), 1, MEMORIA);
        if (read_byte != 1) {
            perror("Error al leer el bitmap de tablas de páginas");
            return -1;
        }

        if (((byte >> bit_index) & 0x01) == 0) { // TPSO libre encontrada
            tps_index = i;
            byte |= (1 << bit_index); // Marcar como ocupada

            fseek(MEMORIA, BITMAP_TPSO_OFFSET + byte_index, SEEK_SET);
            size_t write_byte = fwrite(&byte, sizeof(unsigned char), 1, MEMORIA);
            if (write_byte != 1) {
                perror("Error al actualizar el bitmap de tablas de páginas");
                return -1;
            }
            fflush(MEMORIA);
            break;
        }
    }

    return tps_index;
}