#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 7272

struct account {
	char username[20];
	char password[20];
}account;

int sign_in(int client_socket) {
	
	char message[256];
	
	//rough draft. server would not need to check the size of the username when checking details, only the compare them with records.	
	
	recv(client_socket, account.username, strlen(account.username), 0);

	if (strlen(account.username) > 20) { 
			strcpy(message, "NOTOK");
			send(client_socket, message, sizeof(message), 0);
	}else {
		strcpy(message, "OK");
		send(client_socket, message, sizeof(message), 0);
	}

	recv(client_socket, account.password, strlen(account.password), 0);
	if (sizeof(account.password) > 20) {
		strcpy(message, "NOTOK");
		send(client_socket, message, sizeof(message),0);
	}else {
		strcpy(message, "OK");
		send(client_socket, message, sizeof(message), 0);
	}




}

int create_acc(int client_socket) {

}

void main() { 

	int sockfd;
	struct sockaddr_in server_addr;

	int new_socket;
	struct sockaddr_in new_addr;

	socklen_t addr_size; 
	char message[256] = "Hello there";
	
	system("clear");

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, '\0', sizeof(server_addr));

	if (sockfd == -1) {
		perror("[-] error opening socket ");
	}
	else {
		printf("[+] socket successfully opened\n");
	}

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT);
	server_addr.sin_addr.s_addr= INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("[-] error binding socket to address");
	}
	else { 
		puts("[+] socket successfully bound to port");
	}
	listen(sockfd, 3);
	addr_size = sizeof(new_addr);

	new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
		
	send(new_socket, message, sizeof(message), 0);	

	memset(message, '\0', sizeof(message));

	recv(new_socket, message, sizeof(message), 0);
	if (strncmp(message, "sign_in", 7) == 0) {
		sign_in(new_socket);
	}
		
}

