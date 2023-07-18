#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h> 

#include <string.h>
#include <iostream>
#include <fstream>

#include "config.h"

#include "linked_list.cpp" 

LinkedList requests; // linkedList to store the requests
std::vector<int> accesses(128, 0); // vector to store the accesses per process
pthread_mutex_t mutex; // semáforo mutex de acesso a região crítica
pthread_mutex_t requests_mutex; // mutex semaphore to access requests list
pthread_mutex_t accesses_mutex; // mutex semaphore for writing to the accesses vector
pthread_mutex_t log_mutex; // mutex semaphore for writing to the server log

bool done = false; // Declaration of the global variable "done"

using namespace std;

// Server data structure: keeps track of how many workers are
// active, and whether a shutdown has been requested
typedef struct {
	int port;
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
void *handle_cli(void* arg);

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

void writeToLog(string logMessage){
	ofstream outfile;
	outfile.open("Server_Log.txt", ios::app); // append instead of overwrite
	outfile << logMessage << endl;
	outfile.close();
}

int main(int argc, char **argv)
{
	Server *s = (Server*) malloc(sizeof(Server));
	s->port = SERVER_PORT;

	// Start server thread
	pthread_t server_thr;
	pthread_create(&server_thr, NULL, &server, s);

	// Start handle CLI thread
	pthread_t interface_thr;
    pthread_create(&interface_thr, NULL, &handle_cli, s);

	// Join threads
	pthread_join(server_thr, NULL);
    pthread_join(interface_thr, NULL);

	// Free Server structure
	free(s);

	return 0;
}

// Function executed by the interface thread
void* handle_cli(void* arg) {
    while (true) {
        std::cout << "Digite o comando (1: imprimir fila de pedidos, 2: imprimir quantidade de atendimentos, 3: encerrar): ";

        std::string command;
        std::getline(std::cin, command);

        if (command == "1") {
			pthread_mutex_lock(&requests_mutex);
			requests.print("Fila de pedidos atual: ");
			pthread_mutex_unlock(&requests_mutex);
        } else if (command == "2") {
			pthread_mutex_lock(&accesses_mutex);
            for (int i = 0; i < accesses.size(); i++) {
				if (accesses[i] != 0) {
					std::cout << "Processo " << i << " foi atendido: " << accesses[i] << std::endl;
				}
			}
			pthread_mutex_unlock(&accesses_mutex);
        } else if (command == "3") { // end execution
            done = true;
            break;
        } else {
            std::cout << "Comando inválido! Tente novamente." << std::endl;
        }
    }

    return 0;
}

void *server(void *arg)
{
	Server *s = (Server*) arg;

	// creates socket to listen to new processes connections
	int server_sock_fd;
	server_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock_fd < 0) {
		perror("Couldn't create socket");
		return (void*)-1;
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
		return (void*)-1;
	}

	// Make the server socket nonblocking
	fcntl(server_sock_fd, F_SETFL, O_NONBLOCK);

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(server_sock_fd, &fds);

	rc = listen(server_sock_fd, 5);
	if (rc < 0) {
		perror("Couldn't listen on server socket");
		exit(1);
	}

	while (!done) {
		// Set a 1-second timeout
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		// We want to wait for just the server socket
		fd_set readyfds = fds;

		int rc = select(server_sock_fd+1, &readyfds, NULL, NULL, &timeout);
		if (rc == 1) {
			// The server socket file descriptor became ready,
			// so we can accept a connection
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
			// thread what its client socket fd is, and
			// gives it access to the Server struct)
			Worker *w = (Worker *)malloc(sizeof(Worker));
			w->s = s;
			w->client_fd = client_socket_fd;

			pthread_t worker_thr;
			pthread_create(&worker_thr, NULL, &worker, w);
		}
	}

	close(server_sock_fd);
	return 0;
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
			break; // connection was interrupted
		}

		pthread_mutex_lock(&requests_mutex); // lock access to the requests list
		// gets the required information from the received message
		string receivedMessage = buf;
		int i = receivedMessage.find("|");
        int i2 = (receivedMessage.substr(i+1, receivedMessage.length()-i)).find("|");
        int receveidMessageType = stoi(receivedMessage.substr(0, i));
        int receveidProcessID = stoi(receivedMessage.substr(i+1, i2-i+1));

		if(receveidMessageType == 1){ // Received request
			requests.append(receveidProcessID);

			pthread_mutex_lock(&log_mutex);
			writeToLog("[R] Request -"+to_string(receveidProcessID));
			pthread_mutex_unlock(&log_mutex);

			pthread_mutex_unlock(&requests_mutex); // libera acesso a lista de requests

			// busy wait to enter the critical region until it is the head of the queue
			while (requests.getHeadValue() != receveidProcessID);

			pthread_mutex_lock(&mutex); // process wait to enter in critical region

			// increment number of accesses to the critical region by the process
			accesses[receveidProcessID]++; 
			pthread_mutex_lock(&log_mutex);
			writeToLog("[S] Grant -"+to_string(receveidProcessID));
			pthread_mutex_unlock(&log_mutex);

			string grantMessage = createMessage(2, receveidProcessID);
			fprintf(write_fh, "%s\n", grantMessage.c_str());
			fflush(write_fh);
        }
        if(receveidMessageType == 3){ // Received release
            requests.pop(); // remove the process from the head of the queue, freeing the critical region for the next process

			pthread_mutex_lock(&log_mutex); 
			writeToLog("[R] Release -"+to_string(receveidProcessID));
			pthread_mutex_unlock(&log_mutex);

			pthread_mutex_unlock(&requests_mutex); // release access to the requests list
			pthread_mutex_unlock(&mutex); // release access to the critical region
        }
	}
	printf("Finishing worker...\n");

	// Close the file handles wrapping the client socket
	fclose(read_fh);
	fclose(write_fh);

	// Free Worker struct
	free(w);

	return NULL;
}
