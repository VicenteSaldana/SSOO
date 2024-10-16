#include "../osrms_API/osrms_API.h"
#include "../osrms_File/Osrms_File.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{

    // montar la memoria
    os_mount((char *)argv[1]);

    //resto de instrucciones
    os_ls_processes();
    os_ls_files(115);
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
    // osrmsFile* archivo_w = os_open(115, "probando2", 'w');
    // int bytes = os_write_file(archivo_w, "fuente.txt");
    // os_read_file(archivo_w, "destino.txt");
    // os_close(archivo_w);



    fclose(MEMORIA);
    return 0;

}