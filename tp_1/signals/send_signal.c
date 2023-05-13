#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Erro: número incorreto de argumentos\n");
        printf("Uso correto: %s <pid_destino> <sinal>\n", argv[0]);
        return 1;
    }

    // Converte os argumentos para inteiros 
    pid_t pid_destino = atoi(argv[1]);
    int sinal = atoi(argv[2]);
    
    // Verifica se o processo destino existe
    if (kill(pid_destino, 0) == -1) {
        printf("Erro: processo destino não encontrado\n");
        return 1;
    }

    // Envia o sinal para o processo destino
    kill(pid_destino, sinal);
    printf("Sinal %d enviado para o processo %d\n", sinal, pid_destino);

    return 0;
}