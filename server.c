#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>


#define PORT 7272
#define PENDMAX 3

//THINK ABOUT USING STRTOK TO SEPERATE MESSAGE INTO RECIPIENTS AND CONTENTS

struct account {
	char username[22];
	char password[22];
};

void message_interface(int client_socket) {
	
	//char *buffer;
	//int i;

	//while(1) {
	//	memset(buffer, '\0', strlen(buffer));

	//	recv(client_socket, buffer, strlen(buffer), 0);
		
		/* extract recipient tokens with strtok
		   
		   if 'token for recipients' length is 0
		   	for each connected device 
			send message
		   else if token > 3
		        compare tokens with usernames
			if match
			send out socket connected with selected username
		*/




	
}

int sign_in(int client_socket) {
	
	char *message;
	struct account *temp_acc;

	message = (char *)malloc(256);
	temp_acc = (struct account*)malloc(50);
	
	//rough draft. server would not need to check the size of the username when checking details, only the compare them with records.	
	//ADD WAY OUT OF FUNCTION WHEN CLIENT GETS WRONG
	recv(client_socket, message, sizeof(temp_acc->username), 0);
	recv(client_socket, message, sizeof(temp_acc->username), 0);

	if (strlen(message) > 21) { 
			strcpy(message, "NOTOK");
			send(client_socket, message, sizeof(message), 0);
			printf("user entered incorrect username");
			return -1;
	}else {
		strcpy(temp_acc->username, message);
		send(client_socket, message, strlen(message), 0);
	}	
	recv(client_socket, temp_acc->password, strlen(temp_acc->password), 0);

	if (strlen(message) > 21) {
		strcpy(message, "NOTOK");
		send(client_socket, message, sizeof(message),0);
		return -1;
	}else {
		strcpy(temp_acc->password, message);
	}
	message_interface(client_socket);

	return 0;
}

int create_acc(int client_socket) {
	char *message;
	struct account *new_acc;
	
	message = (char *)malloc(256);
	new_acc = (struct account *)malloc(50);
	//rough draft. server would not need to check the size of the username when checking details, only the compare them with records.	
	//ADD WAY OUT OF FUNCTION WHEN CLIENT GETS WRONG
		recv(client_socket, message, sizeof(new_acc->username), 0);
		

		if (strlen(message) > 20) { 
				strcpy(message, "NOTOK");
				send(client_socket, message, sizeof(message), 0);
				printf("user entered incorrect username");
				return -1;
		}else {
			strcpy(new_acc->username, message);
			strcpy(message, "OK");
			send(client_socket, message, 2, 0);	
		}

		recv(client_socket, new_acc->password, strlen(new_acc->password), 0);

		if (strlen(message) > 20) {
			strcpy(message, "NOTOK");
			send(client_socket, message, sizeof(message),0);
			return -1;
		}else {
			strcpy(new_acc->password, message);
		}
		free(message);
		free(new_acc);
		return 0;

}

int main() { 

	 int sockfd, new_socket, activity;
	 struct sockaddr_in server_addr, new_addr; 
	 char *message;
	 	
	 socklen_t addr_size; 

	 message = (char *)malloc(30);
	 
	 system("clear");

	 sockfd = socket(PF_INET, SOCK_STREAM, 0);

	 memset(&server_addr, '\0', sizeof(server_addr));

	 if (sockfd == -1) {
		 perror("[-] error opening socket ");
	 }
	 else {
		 printf("[+] socket successfully opened\n");
	 }

	 setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

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
	 }
	 else {
 		 printf("[+] listening on port %i...\n", PORT);
	 }
			
	//monitoring multiple clients

	 fd_set readfds;

	 struct timeval select_timeout;
	 memset(&select_timeout, '\0', sizeof(struct timeval));

	 while(1) {
	        FD_ZERO(&readfds);
	        FD_SET(sockfd, &readfds);

	        activity = select(FD_SETSIZE, &readfds, NULL, NULL, &select_timeout);
	        if (activity < 0 && errno != EINTR)
		        continue;

	        if (FD_ISSET(sockfd, &readfds)) {
	       	        addr_size = sizeof(new_addr);

		        if ((new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size)) < 0) {
				if (errno != MSG_DONTWAIT) {	
					continue;
				}
			perror("[-] failed to accept connection ");
			}

		        printf("[+] user has connected on %s\n", inet_ntoa(new_addr.sin_addr));

		        recv(new_socket, message, 10, 0);
		        if (strncmp(message, "sign_in", 7) == 0) {
			       while(sign_in(new_socket) == -1) {
			       sign_in(new_socket);
			       }
		        }
		        else if (strncmp(message, "create_acc", 10) == 0) {
			       while(create_acc(new_socket) == -1) {
			       create_acc(new_socket); 
			       }
			}
		}	
	}
 return 0;	 
}


