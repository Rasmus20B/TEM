#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <errno.h>
#include <signal.h>


#define PORT 7272
#define PENDMAX 3
#define MAXCON 30

//THINK ABOUT USING STRTOK TO SEPERATE MESSAGE INTO RECIPIENTS AND CONTENTS

struct account {
	char username[22];
	char password[22];
	int sockno;
	char ip[INET_ADDRSTRLEN];
};

struct account user[30];

int clients[30];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *message_interface(void *acc) {
	
	char buffer[256];
	struct account *current;

	current = (struct account *)acc;

	while(1) {
		memset(buffer, '\0', strlen(buffer));

		while(recv(current->sockno, buffer, sizeof(buffer), 0) > 0) {
		printf("%s\n", buffer);
		pthread_mutex_lock(&mutex);
		for(int i = 0; i <= MAXCON; i++) {
			if(user[i].username != current->username) {
			send(user[i].sockno, buffer, strlen(buffer), 0);	
			memset(buffer, '\0', sizeof(buffer));
			}
			}
		}
	}
	pthread_mutex_unlock(&mutex);
	
	/* extract recipient tokens with strtok
	   
	   if 'token for recipients' length is 0
	   	for each connected device 
		send message
	   else if token > 3
	        compare tokens with usernames
		if match
		send out socket connected with selected username */

}

int sign_in(int client_socket) {
	
	char *message;
	struct account *temp_acc;

	message = (char *)malloc(256);
	temp_acc = (struct account*)malloc(50);

	pthread_mutex_lock(&mutex);
	
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
	recv(client_socket, temp_acc->password, sizeof(temp_acc->password), 0);

	if (strlen(message) > 21) {
		strcpy(message, "NOTOK");
		send(client_socket, message, sizeof(message),0);
		return -1;
	}else {
		strcpy(temp_acc->password, message);
	}
	pthread_mutex_unlock(&mutex);
	return 0;
}

int create_acc(int client_socket) {
	char *message;
	struct account *new_acc;
	pthread_t message_interface_t;
	
	message = (char *)malloc(256);
	new_acc = (struct account *)malloc(50);

	pthread_mutex_lock(&mutex);
	
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

		recv(client_socket, message, sizeof(new_acc->password), 0);

		if (strlen(message) > 20) {
			strcpy(message, "NOTOK");
			send(client_socket, message, sizeof(message), 0);			
			return -1;
		}else {
			strcpy(new_acc->password, message);
			strcpy(message, "OK");
			send(client_socket, message, sizeof(message), 0);
			for(int i = 0; i <= MAXCON; i++) {
				if(strcmp(user[i].username, "\0") == 0) {
					i++;
			}else {
				strcpy(user[i].username, new_acc->username);
				strcpy(user[i].password, new_acc->password);
			}
		}
	}	
	pthread_mutex_unlock(&mutex);
	free(message);
	pthread_create(&message_interface_t, NULL, (void *)message_interface, (void *)&new_acc);
	free(new_acc);	

	return 0;	
}

void *handle(void *sock) {
	
	char *message;
	struct account user = *((struct account *)sock);

	message = (char *)malloc(15);
	
	recv(user.sockno, message, 10, 0);

	if (strncmp(message, "sign_in", 7) == 0) {
		while(sign_in(user.sockno) == -1) {
			sign_in(user.sockno);
		}
	}else if (strncmp(message, "create_acc", 10) == 0) {
		while(create_acc(user.sockno) == -1) {
			create_acc(user.sockno);
		}
	}
	
	pthread_mutex_unlock(&mutex);

	return 0;

}
int main() { 

	 int sockfd, new_socket; //activity;
	 struct sockaddr_in server_addr, new_addr; 
	 pthread_t sendt, recvt;
	 struct account user;
	 char ip[INET_ADDRSTRLEN];
//	 pid_t childpid;
//	 char *message;
	 	
	 socklen_t addr_size; 

	 //message = (char *)malloc(30);
	 
	 system("clear");

	 sockfd = socket(PF_INET, SOCK_STREAM, 0);

	 memset(&server_addr, '\0', sizeof(server_addr));

	 if (sockfd == -1) {
		 perror("[-] error opening socket ");
	 }
	 else {
		 printf("[+] socket successfully opened\n");
	 }

	 //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

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

//	 fd_set readfds;

	 struct timeval select_timeout;
	 memset(&select_timeout, '\0', sizeof(struct timeval));

	 while(1) {
	////////FD_ZERO(&readfds);
	////////FD_SET(sockfd, &readfds);

	////////activity = select(FD_SETSIZE, &readfds, NULL, NULL, &select_timeout);
	////////if (activity < 0 && errno != EINTR)
	////////        continue;

	////////if (FD_ISSET(sockfd, &readfds)) {
	////////        addr_size = sizeof(new_addr);

	////////        if ((new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size)) < 0) {
	////////		if (errno != MSG_DONTWAIT) {	
	////////			continue;
	////////		}
	////////	perror("[-] failed to accept connection ");
	////////	}

	////////        printf("[+] user has connected on %s\n", inet_ntoa(new_addr.sin_addr));

	////////        recv(new_socket, message, 10, 0);
	////////        if (strncmp(message, "sign_in", 7) == 0) {
	////////	       while(sign_in(new_socket, new_addr) == -1) {
	////////	       sign_in(new_socket, new_addr);
	////////	       }
	////////        }
	////////        else if (strncmp(message, "create_acc", 10) == 0) {
	////////	       while(create_acc(new_socket, new_addr) == -1) {
	////////	       create_acc(new_socket, new_addr); 
	////////	       }
	////////	}
	///////	}	
		 new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
		 if(new_socket < 0) {
			 exit(1);
		 }
		 pthread_mutex_lock(&mutex);	
		 inet_ntop(AF_INET, (struct sockaddr *)&new_addr, ip, INET_ADDRSTRLEN);
		 printf("user has connected on %s\n", inet_ntoa(new_addr.sin_addr));
		 user.sockno = new_socket;
		 strcpy(user.ip, ip);
		 clients[n] = new_socket;
		 n++;
		 pthread_create(&recvt, NULL, handle, &user);
		 pthread_mutex_unlock(&mutex);


	//////// if((childpid = fork()) == 0) {
	////////	 close(sockfd);

	////////	 while(1) {
	////////		 recv(new_socket, message, 10, 0);
	////////		 if(strncmp(message, "sign_in", 7) == 0) {
	////////			 while(sign_in(new_socket, new_addr) == -1) {
	////////				 sign_in(new_socket, new_addr);
	////////			 }
	////////		 }else if (strncmp(message, "create_acc", 10) == 0) {
	////////				 while(create_acc(new_socket, new_addr) == -1) {
	////////					 create_acc(new_socket, new_addr);
	////////				 }
	////////			 
	////////		 
	////////		 }


	////////	}
	//////// }

	 	 
	 }
	 return 0;
}

