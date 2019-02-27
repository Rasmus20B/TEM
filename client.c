#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 7272


/* This program is the client that users interact with */
/* connects to server */

//add quit in messaging function
//
//if message == \quit then
//quit_tem();
//
//else
//buffer = username + message + timestamp
//send buffer
//

int sign_in(int client_socket) {
	
	char username[20];
	char password[20];
	char buffer[20];
	
	//overrides any data stored in buffer
	memset(&buffer, '\0', sizeof(buffer));
	//receives sign in message from server (username)
	recv(client_socket, buffer, 20, 0);

	printf("server: %s\n ", buffer);
	fgets(username, 20, stdin);
	//sends username to server to verify
	send(client_socket, buffer, 20, 0);
	//receives reply
	recv(client_socket, buffer, 20, 0);

	//if reply == NOTOK then 
	if (buffer == "NOTOK") {
		return -1;
	}
	else {
		return 0;
	}
	//receives sign in message from server (password)
	recv(client_socket, buffer, 20, 0);
	printf("server: %s\n ", buffer);
	fgets(password, 20, stdin);
	//sends password to server to verify
	send(client_socket, buffer, 20, 0);
	//recieves reply
	recv(client_socket, buffer, 20, 0);

	//if reply == NOTOK then 
	if (buffer == "NOTOK") {
		return -1;
	}
	else {
		return 0;
	}

}


int connect_to_server(char dest_ip_addr[14]) { //connects to user specified server
	
	int client_socket;
	struct sockaddr_in server_addr;		//initialises a 'sockaddr_in' structure called 'server_addr'
	
	client_socket = socket(PF_INET, SOCK_STREAM, 0);	//creates the TCP client socket
	memset(&server_addr, '\0', sizeof(server_addr));	//overwrites any memory stored in 'server_addr'
	
	server_addr.sin_family=AF_INET;		// specifies family of addresses (IPv4)
	server_addr.sin_port=htons(PORT);	// specifies port the server will be using 
	server_addr.sin_addr.s_addr=inet_addr(dest_ip_addr);	//specifies server IP address received from user

	while (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {	//if the connect function returns -1 (fails)
		perror("[-] error connecting to server ");	//prints an appropriate error message
		return  -1;
	}
	printf("[+] successfully connected\n");

	sign_in(client_socket);

	return 0;
}



int main(int argc, char** argv) {

	char dest_ip_addr[14]; 	//IP address of destination server
	int cor_ip = -1;	// holds the return value of 'connect to server' function

	while (cor_ip ==   -1) {
		if (argv[1] == NULL) { 		// if user has not entered IP address from command line
			puts("please enter the address of the server you would like to join: ");
			fgets(dest_ip_addr, 15, stdin);		// receives server IP address from standard input
			cor_ip = connect_to_server(dest_ip_addr);	//stores the return value of function in cor_ip so it can be reviewed before continuing
			}
		else {		// if user has entered IP address when opening program 
			strcpy(dest_ip_addr, argv[1]);	//copy command line argument to 'dest_ip_addr' variable 
			cor_ip = connect_to_server(dest_ip_addr);	//stores the return value of function in cor_ip so it can be reviewed before continuing
			}	
	}

return 0;
}
	
