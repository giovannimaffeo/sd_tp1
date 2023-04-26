#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 5000
#define BUFFER_SIZE 20

int is_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i < num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(socket_fd, 1);
    int conn_fd = accept(socket_fd, (struct sockaddr*)&client_addr, NULL);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // limpa o buffer antes de cada leitura
        read(conn_fd, buffer, BUFFER_SIZE);
        int num = atoi(buffer);

        printf("Received number: %d\n", num); // apagar

        if (num == 0) {
            break; // consumidor termina o loop ao receber o número 0
        }

        printf("%d is %sprime\n", num, is_prime(num) ? "" : "not ");

        sprintf(buffer, "%d", is_prime(num));
        write(conn_fd, buffer, BUFFER_SIZE);
    }

    close(conn_fd);
}