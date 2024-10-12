#include "../osrms_API/osrms_API.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{

    // montar la memoria
    os_mount((char *)argv[1]);

    //resto de instrucciones
    // os_ls_processes();
    // os_ls_files(234);
    // printf("resultado %i\n", os_exists(234,"secre.txt"));
    // os_frame_bitmap();
    os_tp_bitmap();


    // int bytes = 0xDB;
    // int x;
    // char bit;
    // for (size_t k = 0; k < 8; k++)
    // {
    //     x = 8 - (k+1);
    //     bit = (bytes >> x) & 0x01;
    //     printf("%i\n", bit);
    // }


    fclose(MEMORIA);
    return 0;

}