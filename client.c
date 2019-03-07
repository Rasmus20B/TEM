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
int create_acc() {
	return 0;
}

int sign_in(int client_socket) {
	
	char username[20];
	char password[20];
	char buffer[256];


	puts("please enter your username");
	fgets(username, 25, stdin);
	//sends username to server to verify
	send(client_socket, username, sizeof(username), 0);	
	//receives reply
	recv(client_socket, buffer, sizeof(buffer), 0);

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5)) {
		return -1;
	}
	
	puts("please enter your password");
	fgets(password, 25, stdin);
	//sends password to server to verify
	send(client_socket, password, strlen(password), 0);
	//recieves reply
	recv(client_socket, buffer, sizeof(buffer), 0);

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5)) {
		return -1;
	}else {
		return 0;
		}
}

int connect_to_server(char dest_ip_addr[14]) { //connects to user specified server
	
	int client_socket;
	char buffer[256];	
	int choice = 0;	

	//initialises a 'sockaddr_in' structure called 'server_addr'
	struct sockaddr_in server_addr;		
	
	//creates the TCP client socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);	
	//overwrites any memory stored in 'server_addr'
	memset(&server_addr, '\0', sizeof(server_addr));	
	// specifies family of addresses (IPv4)
	server_addr.sin_family=AF_INET;		
	// specifies port the server will be using
	server_addr.sin_port=htons(PORT);	 
	//specifies server IP address received from user
	server_addr.sin_addr.s_addr=inet_addr(dest_ip_addr);	
	
	//if the connect function returns -1 (fails)
	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {	
		//prints an appropriate error message
		perror("[-] error connecting to server ");	
		return  -1;
	}

	printf("[+] successfully connected\n");
	//recieve welcome message from server along with options
	//print menu
	printf("sign in : 1\fcreate account : 2\f");	
	
	//reinitialise buffer
	memset(buffer, '\0', sizeof(buffer));
		
	//call sign_in function when 1 is selected, call create_account when 2 is selected, else loop
	while (choice == 0) {
		//get choice from user for use in menu
		scanf(" %d", &choice);
		if (choice == 1) {	
			strcpy(buffer, "sign_in");
			send(client_socket, buffer, strlen(buffer), 0); //sends server message to initiate sign in process server-side
			sign_in(client_socket);
		}else if (choice == 2) {	
			strcpy(buffer, "create_acc");
			send(client_socket, buffer, strlen(buffer), 0);
			create_acc(client_socket);
		}else {
			puts("please enter a valid choice");
			choice = 0;
		}
	}
	return 0;
}

void main(int argc, char** argv) {

	//IP address of destination server
	char dest_ip_addr[15]; 	
	// holds the return value of 'connect to server' function
	int cor_ip = -1;	
	
	system("clear");
	
	//copy command line argument to 'dest_ip_addr' variable			
	if (argv[1] != NULL) {
		strcpy(dest_ip_addr, argv[1]);
	}

	while (cor_ip == -1) {	
		// if user has not entered valid IP address from command line
		if (dest_ip_addr == NULL || strlen(dest_ip_addr) < 7 || strlen(dest_ip_addr) > 15) { 		
			puts("please enter the address of the server you would like to join: ");
		
			// receives server IP address from standard input
			fgets(dest_ip_addr, 15, stdin);		
			//stores the return value of 'connect to server' in cor_ip so it can be reviewed before continuing
			cor_ip = connect_to_server(dest_ip_addr);	
			}else {	// if user has entered IP address when opening program 	

			//stores the return value of 'connect_to_server' in cor_ip so it can be reviewed before continuing
			cor_ip = connect_to_server(dest_ip_addr);		
			if (cor_ip == -1) {
				puts("please enter a valid ip address");
			}
			//reintialise memory in dest_ip_address
			memset(dest_ip_addr, '\0', sizeof(dest_ip_addr));		
		}		
	}	
}
	
