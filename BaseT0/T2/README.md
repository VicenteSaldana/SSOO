# Tarea 2

Realizadas todas las funciones pedidas.
Para probarlas se puede ir descomentando el main.c o ir escribiendo las funciones que se quieran probar.
Se genera un leak de memoria al intentar abrir en modo escritura un archivo que ya existe en la memoria (osrmsFile* archivo_w = os_open(115, "existente", 'w');). Fuera de esto no existen más leaks de memoria dentro de la aplicación.