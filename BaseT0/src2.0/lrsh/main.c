
#include <stdio.h>
#include <stdlib.h>
#include "../input_manager/manager.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>





////////////////////
#define EXEC_PROCESS 16
pid_t block_pip;
char **puntero_input = NULL;
typedef struct process{
    pid_t pid;
    char *ejecutable;
    time_t start_t;
    long int tiempo_total;
    int exit_code;
} ProcessInfo;

ProcessInfo procesos[EXEC_PROCESS];
int contador_procesos = 0;
char *input_libre;
/////////////////////
#define MAX_CHILDREN 16
pid_t child_pids[MAX_CHILDREN];
int num_children = 0;
static int keep_running = 1;  // Variable global para controlar el bucle
// Function prototypes
void handle_lrexit();
void handle_hello_command();
void handle_sum_command(float num1, float num2);
void handle_is_prime_command(int num);

void handle_sigint(int sig) {
    // Si se recibe SIGINT (Ctrl + C), se simula la ejecución de lrexit
    keep_running = 0;
    handle_lrexit();
    exit(0);
}

void handle_hello_command() {
    pid_t pid = fork(); 
    if (pid < 0) {
      perror("Error al crear el proceso hijo");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      free_user_input(puntero_input);
      printf("Hello World!\n");
      exit(EXIT_SUCCESS);
    } else {
      block_pip = pid;
      waitpid(pid, NULL, 0);
        // if (num_children < MAX_CHILDREN) {
        //     child_pids[num_children++] = pid;
        // } else {
        //     printf("Error: Número máximo de procesos alcanzado.\n");
        // }
    }
  }

void handle_sum_command(float num1, float num2) {
    pid_t pid = fork(); 
    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
      float result = num1 + num2;
      printf("%f\n", result);
      free_user_input(puntero_input);
      exit(EXIT_SUCCESS);
    } else {
      block_pip = pid;
      waitpid(pid, NULL, 0);
        // if (num_children < MAX_CHILDREN) {
        //     child_pids[num_children++] = pid;
        // } else {
        //     printf("Error: Número máximo de procesos alcanzado.\n");
        // }
    }
}

void handle_is_prime_command(int num) {
    pid_t pid = fork(); 
    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Proceso hijo: calcular si el número es primo
        int is_prime = 1;  // Suponemos que es primo hasta demostrar lo contrario
        if (num <= 1) {
            is_prime = 0;
        } else {
            for (int i = 2; i * i <= num; i++) {
                if (num % i == 0) {
                    is_prime = 0;
                    break;
                }
            }
        }

        if (is_prime) {
            printf("El número %d es primo.\n", num);
        } else {
            printf("El número %d no es primo.\n", num);
        }
        free_user_input(puntero_input);
        exit(EXIT_SUCCESS);
    } else {
      block_pip = pid;
      waitpid(pid, NULL, 0);
        // // Proceso padre: registrar el PID del hijo
        // if (num_children < MAX_CHILDREN) {
        //     child_pids[num_children++] = pid;
        // } else {
        //     printf("Error: Número máximo de procesos alcanzado.\n");
        // }
    }
}

void handle_lrexit(){
  if (input_libre != NULL)
  {
    free(input_libre);
  }
  
  for (size_t i = 0; i < contador_procesos; i++)
  {
    free(procesos[i].ejecutable);
  }
  if (puntero_input != NULL)
  {
    free_user_input(puntero_input);
  }
  keep_running = 0;
      // Enviar SIGINT a todos los procesos hijos
      for (int i = 0; i < num_children; i++) {
          if (child_pids[i] != 0) {  // Si el PID no es 0, el proceso sigue vivo
              kill(child_pids[i], SIGINT);  // Enviar SIGINT

              // Esperar a que el proceso termine
              time_t start_time = time(NULL);
              while (time(NULL) - start_time < 10) {
                  int status;
                  pid_t result = waitpid(child_pids[i], &status, WNOHANG);
                  if (result == 0) {
                      // El proceso aún no ha terminado, seguir esperando
                      sleep(1);
                  } else {
                      // El proceso ha terminado, salir del bucle
                      child_pids[i] = 0;  // Marcar como terminado
                      break;
                  }
              }
              // Si el proceso no ha terminado después de 10 segundos, forzar su terminación
              if (child_pids[i] != 0) {
                  kill(child_pids[i], SIGKILL);
                  waitpid(child_pids[i], NULL, 0);  // Esperar a que el proceso sea eliminado
                  child_pids[i] = 0;  // Marcar como terminado
              }
          }
      }
}



char *copia_string(char *string){
  char *copia = calloc(strlen(string) + 1, sizeof(char));
  if (copia == NULL) {
      perror("calloc failed");
      return NULL;
  }

  strcpy(copia, string);
  return copia;
}

void handle_lrexec_command(char *path, char **argi){
  if (contador_procesos > EXEC_PROCESS)
  {
    return;
  }
  else {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
      printf("Ejecutar %s\n", path);
      execv(path, argi);
      perror("Error ejecutando lrexec");
      free_user_input(puntero_input);
      exit(EXIT_FAILURE);
    }
    else {
        ProcessInfo proces_info = {pid, copia_string(path), time(NULL), 0.0, -1};
        procesos[contador_procesos++] = proces_info;
    }
  }
}

void handle_lrlist_command(){
  pid_t pid = fork();
  if (pid < 0) {
      perror("Error al crear el proceso hijo");
      exit(EXIT_FAILURE);
  }
  else if (pid == 0) {
    free_user_input(puntero_input);
    printf("Hay %i procesos ejecutados por lrexec\n", contador_procesos);
    for (size_t i = 0; i < contador_procesos; i++)
    {
      if (procesos[i].exit_code == -1)
      {
        printf("proceso %li: -pid: %i, -ejecutable: %s, -time: %ld, -exit code: %i\n", i, procesos[i].pid, procesos[i].ejecutable, time(NULL) - procesos[i].start_t,  procesos[i].exit_code);
      }
      else
      {
        printf("proceso %li: -pid: %i, -ejecutable: %s, -time: %li, -exit code: %i\n", i, procesos[i].pid,  procesos[i].ejecutable,  procesos[i].tiempo_total,  procesos[i].exit_code);
      }
    }
    exit(0);
  }
  else {
    block_pip = pid;
    waitpid(pid, NULL, 0);
  }
}

void wait_lrexec(){
  while (1)
  {
    pid_t child_pid;
    int status;
    child_pid = waitpid(-1, &status, WNOHANG);

    if (child_pid == 0 || child_pid == -1) {
      break;
    }
    else {
      if (WIFEXITED(status)) {
          for (size_t i = 0; i < contador_procesos; i++)
          {
            if (child_pid == procesos[i].pid)
            {
              procesos[i].exit_code = WEXITSTATUS(status);
              procesos[i].tiempo_total = time(NULL) - procesos[i].start_t;
              break;
            }
          }
      }
    }
  }
}


int main(int argc, char const *argv[])
{
  signal(SIGINT, handle_sigint);
  while (1)
  {
    wait_lrexec();
    char** input = read_user_input(&input_libre);
    puntero_input = input;
    input_libre = NULL;
    if (strcmp(input[0], "hello") == 0) {
        handle_hello_command();
    }
    else if (strcmp(input[0], "sum") == 0) {
      if (input[1] == NULL || input[2] == NULL) {
          printf("Error: Debes introducir dos números para sumar.\n");
      } else {
          float num1 = atof(input[1]);
          float num2 = atof(input[2]);
          handle_sum_command(num1, num2);
      }
    }
    else if (strcmp(input[0], "is_prime") == 0) {
      if (input[1] == NULL) {
          printf("Error: Debes introducir un número entero.\n");
      } else {
          int num = atoi(input[1]);
          handle_is_prime_command(num);
      }
    }
    else if (strcmp(input[0], "lrexec") == 0) {
      handle_lrexec_command(input[1], &input[1]);
    }
    else if (strcmp(input[0], "lrlist") == 0) {
      wait_lrexec();
      handle_lrlist_command();
    }
    else if (strcmp(input[0], "lrexit") == 0) {
        keep_running = 0;
        handle_lrexit();
        printf("Exiting lrsh...\n");
    }
  free_user_input(input);
  puntero_input = NULL;
  }
}

