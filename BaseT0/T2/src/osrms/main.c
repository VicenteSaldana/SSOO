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
    // os_tp_bitmap();
    // os_start_process(12, "Mono");
    // os_ls_processes();
    // os_frame_bitmap();
    // os_tp_bitmap();
    // os_finish_process(117);
    // os_finish_process(139);
    // os_finish_process(105);
    // os_finish_process(210);
    // os_finish_process(91);
    // os_finish_process(162);
    // os_finish_process(228);
    // os_finish_process(234);
    // os_ls_processes();
    // os_frame_bitmap();
    // os_tp_bitmap();
    // os_ls_processes();



    fclose(MEMORIA);
    return 0;

}