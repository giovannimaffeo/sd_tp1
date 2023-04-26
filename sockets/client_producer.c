#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000
#define BUFFER_SIZE 20

int main(int argc, char* argv[]) {
    int n = atoi(argv[1]); // quantidade de números a serem gerados
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    char buffer[BUFFER_SIZE];
    int num = 1;
    int delta; // declara variável que armazena um valor aleatório entre [1, 100]
    srand(time(0)); // inicializa gerador de números aleatórios
    for (int i = 0; i < n; i++) {  
        delta = rand() % 100 + 1; // gera delta aleatório
        num += delta;
        sprintf(buffer, "%d", num); // converte o número para string de tamanho fixo
        write(socket_fd, buffer, BUFFER_SIZE); 

        sleep(20);
        
        read(socket_fd, buffer, BUFFER_SIZE);
        int is_prime_num = atoi(buffer);

        if (is_prime_num) {
            printf("%d is prime\n", num);
        } else {
            printf("%d is not prime\n", num);
        }
    }

    num = 0;
    sprintf(buffer, "%d", num); // envia o número zero
    write(socket_fd, buffer, BUFFER_SIZE); // escreve no pipe

    close(socket_fd);
}