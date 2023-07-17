// Like server3.c, but uses threads to allow multiple clients
// to connect at the same time.

// Running the server:
//    ./server4 <port>
// where <port> is the port number.

// Use telnet as the client:
//    telnet <port>

#include <unistd.h>
#include <stdio.h> // for perror()
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <netinet/in.h> // for struct sockaddr_in
#include <arpa/inet.h> // for inet_pton

#include <string.h>
#include <iostream>

#include "config.h"
// #define BUFSIZE 2000

// inclusão da classe linkedList desenvolvida
#include "linked_list.cpp" 

LinkedList requests; // linkedList para armazenar as requests
std::vector<int> accesses(128, 0); // vetor para armazenar acessos por cliente
pthread_mutex_t mutex; // semáforo mutex de acesso a região crítica
pthread_mutex_t requests_mutex; // semáforo mutex de acesso a lista de requisições 
//pthread_mutex_t accesses_mutex; // semáforo mutex de acesso a lista de acessos por processo

bool done = false; // Declaração da variável global "done"

using namespace std;
// Server data structure: keeps track of how many workers are
// active, and whether a shutdown has been requested
typedef struct {
	int port;
	int num_workers;
	int shutdown_requested;

	pthread_mutex_t lock;
	pthread_cond_t cond;
} Server;

// Data used by a worker that chats with a specific client
typedef struct {
	Server *s;
	int client_fd;
} Worker;

// Server task - waits for incoming connections, and spawns workers
void *server(void *arg);

// Worker task - chats with clients
void *worker(void *arg);

// Handle CLI task - Command Line Interface
void* handle_cli(void* arg);

string createMessage(int messageType, int process_id){
    /*  Request = 1
        Grant = 2
        Release = 3
    */
    string message = to_string(messageType) + '|' + to_string(process_id) + '|';
    /*  messageType|ID|Padding
    */
    while(size(message)<10)
        message = message + '0';
    return message;
}

int main(int argc, char **argv)
{
	// if (argc != 2) {
	// 	fprintf(stderr, "Usage: ./server4 <port>\n");
	// 	exit(1);
	// }
	Server *s = (Server*) malloc(sizeof(Server));

	// Initialize Server structure
	// s->port = atoi(argv[1]);
	s->port = SERVER_PORT;
	s->num_workers = 0;
	s->shutdown_requested = 0;
	pthread_mutex_init(&s->lock, NULL);
	pthread_cond_init(&s->cond, NULL);

	// Start server thread
	pthread_t thr;
	pthread_create(&thr, NULL, &server, s);

	// Start handle CLI thread
	pthread_t interface_thr;
    pthread_create(&interface_thr, nullptr, &handle_cli, s);

	/*// Wait for shutdown and for workers to finish
	pthread_mutex_lock(&s->lock);
	while (!s->shutdown_requested || s->num_workers > 0) {
		pthread_cond_wait(&s->cond, &s->lock);
	}
	pthread_mutex_unlock(&s->lock);*/

	// Join threads
	pthread_join(thr, nullptr);
    pthread_join(interface_thr, nullptr);

	// Free Server structure
	free(s);

	return 0;
}

// Função que será executada pela thread da interface
void* handle_cli(void* arg) {
    while (true) {
        std::cout << "Digite o comando (1: imprimir fila de pedidos, 2: imprimir quantidade de atendimentos, 3: encerrar): ";

        std::string command;
        std::getline(std::cin, command);

        if (command == "1") {
			pthread_mutex_lock(&requests_mutex); // bloqueia acesso a lista de requests
			requests.print("Fila de pedidos atual: ");
			pthread_mutex_unlock(&requests_mutex); // bloqueia acesso a lista de requests
        } else if (command == "2") {
            for (int i = 0; i < accesses.size(); i++) {
				if (accesses[i] != 0) {
					std::cout << "Processo " << i << " foi atendido: " << accesses[i] << std::endl;
				}
			}
        } else if (command == "3") {
            done = true;
            break;
        } else {
            std::cout << "Comando inválido! Tente novamente." << std::endl;
        }
    }

    return nullptr;
}

void *server(void *arg)
{
	Server *s = (Server*) arg;

	int server_sock_fd;

	server_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock_fd < 0) {
		perror("Couldn't create socket");
		exit(1);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(s->port);
	inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

	int rc = bind(
		server_sock_fd,
		(struct sockaddr *) &server_addr,
		sizeof(server_addr));
	if (rc < 0) {
		perror("Couldn't bind server socket");
		exit(1);
	}

	// Make the server socket nonblocking
	fcntl(server_sock_fd, F_SETFL, O_NONBLOCK);

	// Create a fd_set with the server socket file descriptor
	// (so we can use select to to a timed wait for incoming
	// connections.)
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(server_sock_fd, &fds);

	rc = listen(server_sock_fd, 5);
	if (rc < 0) {
		perror("Couldn't listen on server socket");
		exit(1);
	}

	while (!done) {
		// Use select to see if there are any incoming connections
		// ready to accept

		// Set a 1-second timeout
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		// We want to wait for just the server socket
		fd_set readyfds = fds;

		// Preemptively increase the worker count
		pthread_mutex_lock(&s->lock);
		s->num_workers++;
		pthread_mutex_unlock(&s->lock);

		// Wait until either a connection is received, or the timeout expires
		int rc = select(server_sock_fd+1, &readyfds, NULL, NULL, &timeout);
		// printf("Recebi algo no socket %d\n", rc);
		if (rc == 1) {
			// The server socket file descriptor became ready,
			// so we can accept a connection without blocking
			int client_socket_fd;
			struct sockaddr_in client_addr;
			socklen_t client_addr_size = sizeof(client_addr);

			client_socket_fd = accept(
				server_sock_fd,
				(struct sockaddr*) &client_addr,
				&client_addr_size);
			if (client_socket_fd < 0) {
				perror("Couldn't accept connection");
				exit(1);
			}

			// Create Worker data structure (which tells the worker
			// thread what it's client socket fd is, and
			// gives it access to the Server struct)
			Worker *w = (Worker *)malloc(sizeof(Worker));
			w->s = s;
			w->client_fd = client_socket_fd;

			// Start a worker thread (detached, since no other thread
			// will wait for it to complete)
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			pthread_t thr;
			pthread_create(&thr, &attr, &worker, w);
		} else {
			// The select timed out.

			// Decrement the worker count (since worker wasn't
			// actually started.)
			pthread_mutex_lock(&s->lock);
			s->num_workers--;
			pthread_cond_broadcast(&s->cond);
			pthread_mutex_unlock(&s->lock);

			// See if a shutdown was requested, and if so, stop
			// waiting for connections.
			pthread_mutex_lock(&s->lock);
			if (s->shutdown_requested) {
				done = 1;
			}
			pthread_mutex_unlock(&s->lock);
		}
	}

	close(server_sock_fd);

	return NULL;
}

void *worker(void *arg)
{
	Worker *w = (Worker *)arg;
	Server *s = w->s;

	char buf[BUFFER_SIZE];
	int read_fd = w->client_fd;
	int write_fd = dup(w->client_fd); // duplicate the file descriptor

	// Wrap the socket file descriptor using FILE* file handles,
	// for both reading and writing.
	FILE *read_fh = fdopen(read_fd, "r");
	FILE *write_fh = fdopen(write_fd, "w");

	while (!done) {
		// Read one line from the client
		if (!fgets(buf, BUFFER_SIZE, read_fh)) {
			break; // connection was interrupted?
		}

		pthread_mutex_lock(&requests_mutex); // bloqueia acesso a lista de requests
		//printf("[SERVIDOR] Recebido: %s", buf);

		string receivedMessage = buf;
		int i = receivedMessage.find("|");
        int i2 = (receivedMessage.substr(i+1, receivedMessage.length()-i)).find("|");
        int receveidMessageType = stoi(receivedMessage.substr(0, i));
        int receveidProcessID = stoi(receivedMessage.substr(i+1, i2-i+1));

		if(receveidMessageType == 1){ // Received request
			requests.append(receveidProcessID);
			pthread_mutex_unlock(&requests_mutex); // libera acesso a lista de requests

			while (requests.getHeadValue() != receveidProcessID);

			pthread_mutex_lock(&mutex); // espera para entrar na região crítica

			// incrementa número de acessos a região crítica pela thread 
			accesses[receveidProcessID]++; 

			string grantMessage = createMessage(2, receveidProcessID);
			fprintf(write_fh, "%s\n", grantMessage.c_str());
			fflush(write_fh);
        }
        if(receveidMessageType == 3){ // Received release
            requests.pop();
			pthread_mutex_unlock(&requests_mutex); // libera acesso a lista de requests
			pthread_mutex_unlock(&mutex); // libera o acesso a região crítica
        }
	}

	// Close the file handles wrapping the client socket
	fclose(read_fh);
	fclose(write_fh);

	// Free Worker struct
	free(w);

	// Update worker count
	pthread_mutex_lock(&s->lock);
	s->num_workers--;
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->lock);

	return NULL;
}
