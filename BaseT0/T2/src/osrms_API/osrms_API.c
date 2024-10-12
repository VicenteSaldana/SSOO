#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE* MEMORIA = NULL;

void os_mount(char* memory_path){
    MEMORIA = fopen(memory_path, "r+b");
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
        // printf("\tEstado: 0x%X %d\n", bytes[0], bytes[0]);
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
            // printf("%li\n", ftell(MEMORIA));
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
        // printf("PID: 0x%X %d y process_id %d\n", proceso[1], proceso[1], process_id);
        if (proceso[1] != process_id)
        {
            continue;
        }
        // fseek(MEMORIA, (TAMANO_ENTRADA*i)+13, SEEK_SET);

        for (size_t j = 0; j < CANTIDAD_ARCHIVOS; j++)
        {
            // printf("%li\n", ftell(MEMORIA));
            fread(bytes, sizeof(unsigned char), ARCHIVOS, MEMORIA);
            if (bytes[0] == 0)
            {
                continue;
            }
            // printf("Entrada es valida: 0x%X %d\n", bytes[0], bytes[0]);
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