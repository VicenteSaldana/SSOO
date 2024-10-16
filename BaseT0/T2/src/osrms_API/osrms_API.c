#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE* MEMORIA = NULL;

void os_mount(char* memory_path){
    MEMORIA = fopen(memory_path, "r+b");
    if (MEMORIA == NULL) {
        perror("Error al montar la memoria");
        exit(EXIT_FAILURE);
    }
}

void os_ls_processes(){
    // unsigned char bytes[TAMANO_ENTRADA];        // cambiar por 1+1+11=13
    unsigned char bytes[INFO_PROCESO];
    for (size_t i = 0; i < PROCESO; i++)
    {
        fseek(MEMORIA, TAMANO_ENTRADA*i, SEEK_SET);
        fread(bytes, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (bytes[0] == 0)
        {
            continue;
        }
        char nombre[12];
        strncpy(nombre, (char*)bytes+2, 11);
        nombre[11] = '\0';
        printf("Nombre del Proceso: %s\n", nombre);
        printf("\tEstado: 0x%X %d\n", bytes[0], bytes[0]);
        printf("\tPID: 0x%X %d\n", bytes[1], bytes[1]);
    }
}

int os_exists(int process_id, char* file_name){
    unsigned char bytes[ARCHIVOS];
    unsigned char proceso[INFO_PROCESO];
    for (size_t i = 0; i < PROCESO; i++)
    {
        fseek(MEMORIA, TAMANO_ENTRADA*i, SEEK_SET);
        fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (proceso[0] == 0)
        {
            continue;
        }
        if (proceso[1] != process_id)
        {
            continue;
        }
        for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++)
        {
            printf("%li\n", ftell(MEMORIA));
            fread(bytes, sizeof(unsigned char), ARCHIVOS, MEMORIA);
            if (bytes[0] == 0)
            {
                continue;
            }
            char nombre[15];
            strncpy(nombre, (char*)bytes+1, 14);
            if (!strcmp(nombre, file_name))
            {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

void os_ls_files(int process_id){
    unsigned char bytes[ARCHIVOS];
    unsigned char proceso[INFO_PROCESO];
    for (size_t i = 0; i < PROCESO; i++)
    {
        fseek(MEMORIA, TAMANO_ENTRADA*i, SEEK_SET);
        fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (proceso[0] == 0)
        {
            continue;
        }
        printf("PID: 0x%X %d y process_id %d\n", proceso[1], proceso[1], process_id);
        if (proceso[1] != process_id)
        {
            continue;
        }
        // fseek(MEMORIA, (TAMANO_ENTRADA*i)+13, SEEK_SET);

        for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++)
        {
            printf("%li\n", ftell(MEMORIA));
            fread(bytes, sizeof(unsigned char), ARCHIVOS, MEMORIA);
            if (bytes[0] == 0)
            {
                continue;
            }
            printf("Entrada es valida: 0x%X %d\n", bytes[0], bytes[0]);
            char nombre[15];
            strncpy(nombre, (char*)bytes+1, 14);
            nombre[14] = '\0';
            printf("Nombre archivo: %s\n", nombre);
            char tamaño[4];
            strncpy(tamaño, (char*)bytes+15, 4);
            int numero;
            memcpy(&numero, tamaño, sizeof(int));
            printf("\tTamaño archivo: %i Bytes\n", numero);
        }
        break;
    }
}

void os_frame_bitmap(){
    unsigned char bytes[LECTURA_FRAME];
    int x;
    char bit;
    unsigned int contador = 0;
    unsigned int ocupado = 0;
    unsigned int disponible = 0;
    fseek(MEMORIA, FRAME_BITMAP_M, SEEK_SET);
    for (size_t i = 0; i < COLUMA_FRAME; i++)
    {
        fread(bytes, sizeof(unsigned char), LECTURA_FRAME, MEMORIA);
        for (size_t j = 0; j < LECTURA_FRAME; j++)
        {
            for (size_t k = 0; k < 8; k++)
            {
                x = 8 - (k+1);
                bit = (bytes[j] >> x) & 0x01;
                printf("%i ", bit);
                if (bit == 1)
                {
                    ocupado++;
                }
                else
                {
                    disponible++;
                }
                contador++;
            }
        }
        printf("\n");
    }
    printf("contador: %u\n", contador);
    printf("Ocupado: %u\n", ocupado);
    printf("Disponible: %u\n", disponible);
}

void os_tp_bitmap(){
    unsigned char bytes[LECTURA_BITMAP];
    int x;
    char bit;
    unsigned int contador = 0;
    unsigned int ocupado = 0;
    unsigned int disponible = 0;
    fseek(MEMORIA, BITMAP_T_PAGINAS_M, SEEK_SET);
    for (size_t i = 0; i < COLUMA_BITMAP; i++)
    {
        fread(bytes, sizeof(unsigned char), LECTURA_BITMAP, MEMORIA);
        for (size_t j = 0; j < LECTURA_BITMAP; j++)
        {
            for (size_t k = 0; k < 8; k++)
            {
                x = 8 - (k+1);
                bit = (bytes[j] >> x) & 0x01;
                printf("%i ", bit);
                if (bit == 1)
                {
                    ocupado++;
                }
                else
                {
                    disponible++;
                }
                contador++;
            }
        }
        printf("\n");
    }
    printf("contador: %u\n", contador);
    printf("Ocupado: %u\n", ocupado);
    printf("Disponible: %u\n", disponible);
}

void os_start_process(int process_id, char* process_name){
    unsigned char proceso[INFO_PROCESO];
    long int lugar_vacio = -1;
    long int lugar_actual;
    unsigned char escribir[INFO_PROCESO];
    escribir[0] = 0x01;
    escribir[1] = process_id;
    strncpy((char*)&escribir[2], process_name, 11);
    for (int j = 2 + strlen(process_name); j < 13; j++) {
        escribir[j] = 0;
    }

    for (size_t i = 0; i < PROCESO; i++)
    {
        fseek(MEMORIA, TAMANO_ENTRADA*i, SEEK_SET);
        lugar_actual = ftell(MEMORIA);
        fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (proceso[0] == 0)
        {
            if (lugar_vacio == -1)
            {
                lugar_vacio = lugar_actual;
            }
            continue;
        }
        if (proceso[1] == process_id)
        {
            printf("PID OCUPADO\n");
            return;
        }
    }
    fseek(MEMORIA, lugar_vacio, SEEK_SET);
    fwrite(escribir, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
}

// SUPUESTO: Un frame ocupado es un frame lleno y una pagina ocupada es una pagina llena
void os_finish_process(int process_id){
    unsigned char bytes[ARCHIVOS];
    unsigned char proceso[INFO_PROCESO];
    unsigned char tp_bitmap[2];
    unsigned char tp_frame[2];
    unsigned char byte_frame[1];
    unsigned char byte_bitmapPO[1];
    unsigned char byte_zero[1];
    byte_zero[0] = 0x00; 
    long int lugar_proceso;
    long int lugar_archivo;
    unsigned int VPN;
    unsigned int TPPO;
    unsigned int D_PFN;
    unsigned int byte_bitmap;
    unsigned char bit_bitmpa;
    unsigned int numero_tp;
    unsigned int frame;
    unsigned int bit_frame;
    unsigned int bit_bitmapPO;
    unsigned int offset;
    unsigned int direccion_virtual;
    int tamano_archivo;
    int tamano_frame = 32768;
    for (size_t i = 0; i < PROCESO; i++)
    {
        fseek(MEMORIA, TAMANO_ENTRADA*i, SEEK_SET);
        lugar_proceso = ftell(MEMORIA);
        fread(proceso, sizeof(unsigned char), INFO_PROCESO, MEMORIA);
        if (proceso[0] == 0)
        {
            continue;
        }
        if (proceso[1] != process_id)
        {
            continue;
        }
        for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++)
        {
            // OBTENCION del VPN y offset
            printf("COMIENZO NUEVO ARCHIVO: %li linea: %li\n",j, ftell(MEMORIA));
            lugar_archivo = ftell(MEMORIA);
            fread(bytes, sizeof(unsigned char), ARCHIVOS, MEMORIA);
            if (bytes[0] == 0)
            {
                continue;
            }
            // char tamaño[4];
            // strncpy(tamaño, (char*)bytes+15, 4);
            // tamano_archivo = 0;
            // memcpy(&tamano_archivo, tamaño, sizeof(int));
            tamano_archivo = 0; 
            tamano_archivo = (bytes[18] << 24) | tamano_archivo;
            tamano_archivo = (bytes[17] << 16) | tamano_archivo;
            tamano_archivo = (bytes[16] << 8) | tamano_archivo;
            tamano_archivo = (bytes[15]) | tamano_archivo;
            // char direccion_virtual[4];
            // strncpy(direccion_virtual, (char*)bytes+19, 4);
            direccion_virtual = (bytes[22] << 24) | (bytes[21] << 16) | (bytes[20] << 8) | bytes[19];
            printf("1: 0x%X, 2: 0x%X, 3: 0x%X, 4: 0x%X\n", bytes[19], bytes[20], bytes[21], bytes[22]);
            VPN = 0;
            VPN = (direccion_virtual >> 15) & 0xFFF;;
            printf("VPN:0x%X\n", VPN);
            offset = 0;
            offset = direccion_virtual & 0x7FFF;
            printf("offset: 0x%X\n", offset);

            // 
            //////////////////////////
            while (tamano_archivo > 0)
            {
                // OBTENCION de la entrada del Bitmap de Tablas de Paginas y del Frame bitmap
                printf("tamano_archivo: %i, 0x%X\n", tamano_archivo, tamano_archivo);
                TPPO = 0;
                TPPO = VPN >> 6;
                printf("TPPO:0x%X\n", TPPO);
                fseek(MEMORIA, (lugar_proceso + TABLA_PAGINA_PO +(2*TPPO)), SEEK_SET);
                fread(tp_bitmap, sizeof(unsigned char), 2, MEMORIA);
                numero_tp = 0;
                numero_tp = tp_bitmap[1] << 8 | numero_tp;
                numero_tp = tp_bitmap[0] | numero_tp;
                printf("0x%X, 0x%X, 0x%X\n", tp_bitmap[0], tp_bitmap[1], numero_tp);
                printf("numero de la Tabla de Paginas de Segundo Orden: %u, 0x%X\n", numero_tp, numero_tp);
                fseek(MEMORIA, ESPACIO_TPSO, SEEK_SET);
                fseek(MEMORIA, TAMANO_TABLA_TPSO*numero_tp, SEEK_CUR);
                // Tabla de Paginas de Segundo Orden
                D_PFN = 0;
                D_PFN = VPN & 0x0000003F;
                printf("VPN 0x%X, D_PFN 0x%X\n", VPN, D_PFN);
                fseek(MEMORIA, (D_PFN*2), SEEK_CUR);
                fread(tp_frame, sizeof(unsigned char), 2, MEMORIA);
                printf("DONDE %li\n", ftell(MEMORIA));
                frame = 0;
                frame = tp_frame[1] << 8 | frame;
                frame = tp_frame[0] | frame;

                tamano_archivo -= (tamano_frame-offset);    //32KB
                VPN++;
                offset = 0;
                // MODIFICACION  Frame bitmap
                byte_bitmap = 0;
                bit_bitmpa = 0;
                byte_bitmap = frame / 8;
                bit_bitmpa = frame % 8;
                printf("%i, %i\n", byte_bitmap, bit_bitmpa);
                fseek(MEMORIA, (FRAME_BITMAP_M+byte_bitmap), SEEK_SET);
                fread(byte_frame, sizeof(unsigned char), 1, MEMORIA);
                bit_frame = 0;
                bit_frame = (byte_frame[0] >> (7-bit_bitmpa)) & 0x01;
                printf("bit_frame %i\n", bit_frame);
                printf("ANTES 0x%X\n", byte_frame[0]);
                byte_frame[0] = ~((0x01 << (7-bit_bitmpa))) & byte_frame[0];
                printf("DESPUES 0x%X\n", byte_frame[0]);
                fseek(MEMORIA, (FRAME_BITMAP_M+byte_bitmap), SEEK_SET);
                fwrite(byte_frame, sizeof(unsigned char), 1, MEMORIA);
                fseek(MEMORIA, (FRAME_BITMAP_M+byte_bitmap), SEEK_SET);
                fread(byte_frame, sizeof(unsigned char), 1, MEMORIA);
                bit_frame = 0;
                bit_frame = (byte_frame[0] >> (7-bit_bitmpa)) & 0x01;
                printf("AHORA bit_frame %i\n", bit_frame);
                printf("AHORA 0x%X\n", byte_frame[0]);

                // MODIFICACION  Bitmap de Tablas de Paginas
                byte_bitmap = 0;
                bit_bitmpa = 0;
                byte_bitmap = numero_tp / 8;
                bit_bitmpa = numero_tp % 8;
                fseek(MEMORIA, (BITMAP_T_PAGINAS_M+byte_bitmap), SEEK_SET);
                fread(byte_bitmapPO, sizeof(unsigned char), 1, MEMORIA);
                bit_bitmapPO = 0;
                bit_bitmapPO = (byte_bitmapPO[0] >> (7-bit_bitmpa)) & 0x01;
                printf("bit_bitmapPO %i\n", bit_bitmapPO);
                printf("ANTES 0x%X\n", byte_bitmapPO[0]);
                byte_bitmapPO[0] = ~((0x01 << (7-bit_bitmpa))) & byte_bitmapPO[0];
                printf("DESPUES 0x%X\n", byte_bitmapPO[0]);
                fseek(MEMORIA, (BITMAP_T_PAGINAS_M+byte_bitmap), SEEK_SET);
                fwrite(byte_bitmapPO, sizeof(unsigned char), 1, MEMORIA);
                fseek(MEMORIA, (BITMAP_T_PAGINAS_M+byte_bitmap), SEEK_SET);
                fread(byte_bitmapPO, sizeof(unsigned char), 1, MEMORIA);
                bit_bitmapPO = 0;
                bit_bitmapPO = (byte_bitmapPO[0] >> (7-bit_bitmpa)) & 0x01;
                printf("AHORA bit_bitmapPO %i\n", bit_bitmapPO);
                printf("AHORA 0x%X\n", byte_bitmapPO[0]);
            }
            //////////////////////////

            // MODIFICACION  Entrada Archivo
            fseek(MEMORIA, lugar_archivo, SEEK_SET);
            fwrite(byte_zero, sizeof(unsigned char), 1, MEMORIA);

            fseek(MEMORIA, (lugar_archivo + ARCHIVOS), SEEK_SET);
            printf("\n");
        }
        // MODIFICACION  Entrada Proceso
        fseek(MEMORIA, lugar_proceso, SEEK_SET);
        fwrite(byte_zero, sizeof(unsigned char), 1, MEMORIA);
        return;
    }
    printf("No se encontro el proceso en la tabla de PCBs\n");
}
