#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ostream>

#include "config.h"

using namespace std;

int process_id = -99;

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

using time_point = std::chrono::system_clock::time_point;
string formatTime( const time_point& time){
    string format = "%d/%m/%Y - %H:%M:%S";
    time_t tt = chrono::system_clock::to_time_t(time);
    tm tm = *localtime(&tt); //Locale time-zone, usually UTC by default.
    stringstream ss;
    ss << put_time( &tm, format.c_str() );

    auto ms = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch()) - chrono::duration_cast<chrono::seconds>(time.time_since_epoch());
    ostringstream oss;
    // Set the fill character to '0'
    oss << setfill('0');
    oss << setw(3);
    // Append the number with leading zeros to the string
    oss << ms.count();
    string ms_str = oss.str();
    
    string time_formatted = ss.str() + "." + ms_str;
    return time_formatted;
}

int main(int argc, char *argv[]){
    // pid_t child = fork();
    process_id = atoi(argv[1]);
    int r = atoi(argv[2]); // Número de requisições à região crítica
    int k = atoi(argv[3]); // Tempo em segundos que o cliente deve dormir após escrever no arquivo
    int test_index = atoi(argv[4])
    
	int rc;

	int write_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (write_fd < 0) {
		perror("Could not create socket");
		exit(1);
	}
    printf("Socket criado.\n");

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

	rc = connect(write_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (rc < 0) {
		perror("Could not connect to server");
		exit(1);
	}
    printf("Conectado ao servidor.\n");

    int read_fd = dup(write_fd); 

    FILE *write_fh = fdopen(write_fd, "w");
	FILE *read_fh = fdopen(read_fd, "r");

    // pid_t x = fork();

    // if(x==0)
    //     process_id = 33;

    ofstream outfile;
    string requestMessage = createMessage(1, process_id);
    string releaseMessage = createMessage(3, process_id);
    char buffer[BUFFER_SIZE];
    for(int i=0; i<r; i++){
        cout << "[CLIENTE " << process_id << "] Enviando request ao servidor..." << endl;
	    fprintf(write_fh, "%s\n", requestMessage.c_str());
		fflush(write_fh);

        // Read one line from the server
		if (!fgets(buffer, BUFFER_SIZE, read_fh)) {
			break; // connection was interrupted?
		}

        cout << "[CLIENTE " << process_id << "] Recebido: " << buffer;

        // Escreve hora no arquivo Resultados 
        auto now = chrono::system_clock::now();
        string formatted_time = formatTime(now);
        outfile.open("Resultados_" + test_index + ".txt", ios::app); // append instead of overwrite
        outfile << "[" << process_id << "] " << formatted_time << endl;
        outfile.close();
        sleep(k);
        
        cout << "[CLIENTE " << process_id << "] Enviando release ao servidor..." << endl;
        fprintf(write_fh, "%s\n", releaseMessage.c_str());
		fflush(write_fh);
    }

    fclose(read_fh);
	fclose(write_fh);
    return 0;
}