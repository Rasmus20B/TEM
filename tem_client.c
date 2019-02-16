#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* This program is the client that users interact with */

#define PORT 7272
int connect_to_server(char dest_ip_addr[15]) { //connects to user specified server
	
int client_socket;
struct sockaddr_in server_addr;		//initialises a 'sockaddr_in' structure called 'server_addr'
char buffer[256];	//used for storing messages before displaying to screen or sending to server

system("clear");

client_socket = socket(PF_INET, SOCK_STREAM, 0);	//creates the TCP client socket
memset(&server_addr, '\0', sizeof(server_addr));	//overwrites any memory stored in 'server_addr' with zeros
	
server_addr.sin_family=AF_INET;		// specifies family of addresses (IPv4)
server_addr.sin_port=htons(PORT);	// specifies port the server will be using 
server_addr.sin_addr.s_addr=inet_addr(dest_ip_addr);	//specifies server IP address received from user

if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {	//if the connect function returns -1 (fails)
	perror("[-] error connecting to server ");	//prints an appropriate error message
}
else {		//if the connect function returns 0 (succeeds)
	printf("[+] successfully connected\n");
	recv(client_socket, buffer, 256, 0);		//recieves message from server and stores it in buffer
	printf("[+] Server : %s\n ", buffer);		//prints the server message stored in buffer

	}

close(client_socket);		//closes the open socket connected to the server

		return 0;
}

int main(int argc, char** argv) {

char dest_ip_addr[15];

if (argv[1] == NULL) { 		// if user has not entered IP address from command line
puts("please enter the address of the server you would like to join\n\n\n\n");
fgets(dest_ip_addr, 15, stdin);		// receives server IP address from standard input
}
else {		// if user has entered IP address when opening program 
	strcpy(dest_ip_addr, argv[1]);	//copy command line argument to 'dest_ip_addr' variable 
}
connect_to_server(dest_ip_addr);
}
	
