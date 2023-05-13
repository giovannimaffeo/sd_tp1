#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h" // importa variáveis globais do arquivo de configuração

// função que verifica se um número é primo
int is_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i < num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

int main() {
    // cria um socket e salva o descritor em socket_fd
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    // define as informações do endereço do servidor
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // associa o socket criado com as informações de endereço do servidor
    bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    // configura o socket para receber conexões de clientes e definimos o tamanho máximo da fila de conexões pendentes
    listen(socket_fd, 1);

    // aguarda a conexão do cliente e salva o descritor da conexão em conn_fd
    printf("Consumidor: aguardando conexão...\n"); 
    int conn_fd = accept(socket_fd, (struct sockaddr *) NULL, NULL);
    printf("Consumidor: conexão estabelecida\n"); 

    char buffer[BUFFER_SIZE]; // declara string de tamanho fixo que armazena as mensagens
    // loop infinito que recebe números do produtor e verifica primalidade
    while(1) {
        read(conn_fd, buffer, BUFFER_SIZE); // lê o número enviado pelo produtor e salva em buffer
        int num = atoi(buffer);

        if (num == 0) {
            break; // consumidor termina o loop ao receber o número 0
        }

        printf("Consumidor: recebido número %s do produtor\n", buffer);
        sprintf(buffer, "%d", is_prime(num));
        write(conn_fd, buffer, BUFFER_SIZE); // envia o resultado da verificação de primalidade para o produtor
    }

    close(conn_fd); // fecha o socket de conexão com o produtor
    close(socket_fd); // fecha o socket em modo de escuta por novas conexões

    return 0;
}