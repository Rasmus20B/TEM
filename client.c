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

//ACCOUNT STRUCTURE TO FIX SPAGHET

//add quit in messaging function
//
//if message == \quit then
//quit_tem();
//
//else
//buffer = username + message + timestamp
//send buffer
//
struct account {
	char username[20];
	char password[20];
};

void message_interface(char username, int client_socket)  {
	
	char *buffer;
	int i;

	buffer = (char *)malloc(256);

	while(1) {
		memset(buffer, '\0', strlen(buffer));
		puts("message : ");
		while ((buffer[i++] = getchar()) != '\n') 
		;

		send(client_socket, buffer, strlen(buffer), 0);
		memset(buffer, '\0', strlen(buffer));
		recv(client_socket, buffer, strlen(buffer), 0);
		printf("%s\n", buffer);
		if ((strcmp(buffer, "exit")) == 0) {
			puts("Exiting...\n");
			break;
		
		}
		
	}
}

int create_acc(int client_socket) { //WORK ON THIS ONE FIRST
	char *buffer;
	struct account *new_acc;
	
	//allocate memory for buffer and account structure 'new_acc'
	new_acc = (struct account*)malloc(sizeof(struct account));		
	buffer = (char*)malloc(sizeof(char)*256);

	puts("please enter your username\n"); 
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin);
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin);
	//sends username to server to verify
	send(client_socket,new_acc->username, strlen(new_acc->username), 0);	
	//receives reply
	recv(client_socket, buffer, strlen(buffer), 0);
	for (int i = 0; i <= 20; ++i) {
		if (buffer[i] == '\n') {
			buffer[i] = '\0';
		}
	}

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5)) {
		puts("invalid username\n");
		//free memory and return function failure
		free(new_acc->username);
		return -1;
	}
	else if (strncmp(buffer, "OK", 2)) {
	
		puts("please enter your password\n");
		fgets(new_acc->password, 22, stdin);
		//sends password to server to verify
		send(client_socket, new_acc->password, strlen(new_acc->password), 0);
		//recieves reply
		recv(client_socket, buffer, sizeof(buffer), 0);

		//if reply == NOTOK then 
		if (strncmp(buffer, "NOTOK", 5)) {
			printf("incorrect password");
			free(new_acc->password);
			return -1;
		//if reply == OK then
		}else {
			//free memory and call messaging interface
			free(new_acc->password);
			free(buffer);
			message_interface(*new_acc->username, client_socket);
		
		}
	}
	return 0;
}

int sign_in(int client_socket) { //NOT SYNCHRONOUS WITH SERVER, ADD A WAY OUT OF THE FUNCTION FOR SERVER
		
	char buffer[256];
	struct account *new_acc;

	new_acc = (struct account*)malloc(sizeof(struct account));	

	printf("\nplease enter your username : ");
	fgets(new_acc->username, sizeof(new_acc->username), stdin);
	//sends username to server to verify
	send(client_socket,new_acc->username, strlen(new_acc->username), 0);	
	//receives reply
	recv(client_socket, buffer, strlen(buffer), 0);

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5)) {
		puts("invalid username\n");
		free(new_acc->username);
		return -1;
	}
	
	puts("please enter your password\n");
	fgets(new_acc->password, sizeof(new_acc->password), stdin);
	//sends password to server to verify
	send(client_socket, new_acc->password, strlen(new_acc->password), 0);
	//recieves reply
	recv(client_socket, buffer, sizeof(buffer), 0);

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5)) {
		printf("incorrect password");
		free(new_acc->password);
		return -1;
	}else {
		free(new_acc->password);
		return 0;
	}
}

int connect_to_server(char dest_ip_addr[14]) { //connects to user specified server
	
	int client_socket;
	char *buffer;	
	int choice = 0;	

	//initialises a 'sockaddr_in' structure called 'server_addr'
	struct sockaddr_in server_addr;		

	buffer = (char *)malloc(256);
	
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
	printf("sign in : 1\ncreate account : 2\njoin as guest : 3\n");	
		
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

	free(buffer);
	return 0;

}

int main(int argc, char** argv) {

	//IP address of destination server
	char *dest_ip_addr; 	
	// holds the return value of 'connect to server' function
	int cor_ip = -1;	
	//clear terminal screen	
	system("clear");
	
	//allocate memory to dest_ip_addr
	dest_ip_addr = (char *)malloc(15);

	//copy command line argument to 'dest_ip_addr' variable			
	if (argv[1] != NULL) {
		strcpy(dest_ip_addr, argv[1]); //SECURITY FLAW
	}
	
	//while connect function fails
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
			//reintialise memory in dest_ip_address that has been written to
			memset(dest_ip_addr, '\0', strlen(dest_ip_addr));		
		}		
	}
	//frees the memory allocated to dest_ip_address
	free(dest_ip_addr);	
	return 0;
}
	
