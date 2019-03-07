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
#define PENDMAX 3

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

	 int sockfd, new_socket, activity;
	 struct sockaddr_in server_addr; 
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

	 if (listen(sockfd, PENDMAX) < 0) {
		 perror("[-] error listening ");
		 close(sockfd);
	 }
	 else {
 		 printf("[+] listening to on port %i...\n", PORT);
	 }
			
	//monitoring multiple clients

	 fd_set readfds;

	 struct timeval select_timeout;
	 memset(&select_timeout, '\0', sizeof(struct timeval));

	 while(1) {
	        FD_ZERO(&readfds);
	        FD_SET(sockfd, &readfds);

	        activity = select(FD_SETSIZE, &readfds, NULL, NULL, &select_timeout);
	        if (activity < 0)
		        continue;

	        if (FD_ISSET(sockfd, &readfds)) {
	       	        addr_size = sizeof(new_addr);
		        if ((new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size)) < 0) {
			        perror("[-] failed to accept connection ");
		        }
		        printf("[+] user %s has connected \n", inet_ntoa(new_addr.sin_addr));
		        recv(new_socket, message, 9, 0);
		        if (strncmp(message, "sign_in", 7) == 0) {
			       sign_in(new_socket);
		        }
		        else if (strncmp(message, "create_acc", 12) == 0) {
			       create_acc(new_socket);
		       
			}
	 	       
	       }	
	}
}

