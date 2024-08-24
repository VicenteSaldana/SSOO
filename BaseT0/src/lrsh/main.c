
#include <stdio.h>
#include <stdlib.h>
#include "../input_manager/manager.h"
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <string.h>
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
        printf("Hello World!\n");
        exit(EXIT_SUCCESS);
    } else {
        if (num_children < MAX_CHILDREN) {
            child_pids[num_children++] = pid;
        } else {
            printf("Error: Número máximo de procesos alcanzado.\n");
        }
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
        exit(EXIT_SUCCESS);
    } else {
        if (num_children < MAX_CHILDREN) {
            child_pids[num_children++] = pid;
        } else {
            printf("Error: Número máximo de procesos alcanzado.\n");
        }
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
        exit(EXIT_SUCCESS);
    } else {
        // Proceso padre: registrar el PID del hijo
        if (num_children < MAX_CHILDREN) {
            child_pids[num_children++] = pid;
        } else {
            printf("Error: Número máximo de procesos alcanzado.\n");
        }
    }
}

void handle_lrexit(){
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

int main(int argc, char const *argv[])
{
  signal(SIGINT, handle_sigint);
  while (keep_running) {
    char** input = read_user_input();
    // Si el comando es lrexit, salir del bucle
    if (strcmp(input[0], "hello") == 0) {
        handle_hello_command();
    }
    else if (strcmp(input[0], "lrexit") == 0) {
        keep_running = 0;
        handle_lrexit();
        printf("Exiting lrsh...\n");
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
  free_user_input(input);
  }
}
