#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void handle_sigint(int sig) {
    printf("Recebido sinal SIGINT\n");
    printf("Finalizando processo…\n");
    exit(0);
}

void handle_sigterm(int sig) {
    printf("Recebido sinal SIGTERM\n");
}

void handle_sigusr1(int sig) {
    printf("Recebido sinal SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Erro: número incorreto de argumentos\n");
        printf("Uso correto: %s <tipo_espera>\n", argv[0]);
        return 1;
    }

    // Define os signal handlers para SIGINT, SIGTERM e SIGUSR1
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);

    int wait_type = -1; // Inicializa com um valor inválido
    if (strcmp(argv[1], "blocking_wait") == 0) {
        wait_type = 0;
    } else if (strcmp(argv[1], "busy_wait") == 0) {
        wait_type = 1;
    }

    // Aguarda sinais
    if (wait_type == 0) {
        // Blocking wait
        while (1) {
            pause();
        }
    } else if (wait_type == 1) {
        // Busy wait
        while (1) {}
    } else {
        printf("Erro: tipo de espera inválido\n");
        printf("Opções válidas: blocking_wait, busy_wait\n");
        return 1;
    }

    return 0;
}