#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#include "config.h" // importa variáveis globais do arquivo de configuração

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("Erro: número incorreto de argumentos\n");
        printf("Uso correto: %s <quantidade_numeros_a_gerar>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]); // quantidade de números a serem gerados

    // cria um socket e salva o descritor em socket_fd
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    // define as informações do endereço do servidor
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // conecta ao servidor usando as informações de endereço definidas acima
    connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    char buffer[BUFFER_SIZE]; // declara string de tamanho fixo que armazena as mensagens
    int num = 1; // define N0 com o valor 1
    int delta; // declara variável que armazena um valor aleatório entre [1, 100]
    srand(time(0)); // inicializa gerador de números aleatórios

    for (int i = 0; i < n; i++) {  
        delta = rand() % 100 + 1; // gera delta aleatório
        num += delta;
        sprintf(buffer, "%d", num); // converte o número para string de tamanho fixo
        write(socket_fd, buffer, BUFFER_SIZE); // envia o número gerado para o consumidor

        read(socket_fd, buffer, BUFFER_SIZE); // lê a primalidade enviada pelo consumidor e salva em buffer
        int is_prime_num = atoi(buffer);
        if (is_prime_num) {
            printf("Produtor: %d é primo\n", num);
        } else {
            printf("Produtor: %d não é primo\n", num);
        }
    }

    num = 0;
    sprintf(buffer, "%d", num); 
    write(socket_fd, buffer, BUFFER_SIZE); // escreve o número zero para o consumidor

    close(socket_fd); // fecha o socket de conexão com o servidor

    return 0;
}