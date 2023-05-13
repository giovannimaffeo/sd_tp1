#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 20

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("Erro: número incorreto de argumentos\n");
        printf("Uso correto: %s <quantidade_numeros_a_gerar>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]); // quantidade de números a serem gerados

    int fd[2]; // declara array que armazena os descritores
    char buffer[BUFFER_SIZE]; // declara string de tamanho fixo que armazena as mensagens

    pid_t childpid; // declara variável que armazena o pid do processo filho

    int num = 1; // define N0 com o valor 1
    int delta; // declara variável que armazena um valor aleatório entre [1, 100]
    srand(time(0)); // inicializa gerador de números aleatórios
    
    pipe(fd); // cria o pipe   
    childpid = fork(); // cria o processo filho
    
    if (childpid != 0) { // processo pai é o produtor
        close(fd[0]); // fecha a ponta de leitura do pipe
        
        for (int i = 0; i < n; i++) {  
            delta = rand() % 100 + 1; // gera delta aleatório
            num += delta;
            sprintf(buffer, "%d", num); // converte o número para string de tamanho fixo
            write(fd[1], buffer, BUFFER_SIZE); // escreve no pipe
        }
        
        num = 0;
        sprintf(buffer, "%d", num); // envia o número zero
        write(fd[1], buffer, BUFFER_SIZE); // escreve no pipe
        
        close(fd[1]); // fecha a ponta de escrita do pipe
    }
    else { // processo filho é o consumidor
        close(fd[1]); // fecha a ponta de escrita do pipe
        
        while (1) {
            read(fd[0], buffer, BUFFER_SIZE); // lê do pipe
            num = atoi(buffer); // converte a string para número
            
            if (num == 0) {
                break; // consumidor termina o loop ao receber o número 0
            }
                        
            if (is_prime(num)) {
                printf("%d é primo\n", num);
            } else {
                printf("%d não é primo\n", num);
            }
        }
        
        close(fd[0]); // fecha a ponta de leitura do pipe
    }
    
    return 0;
}