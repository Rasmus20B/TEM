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
	char ip[INET_ADDRSTRLEN];
	int sockno;	
};

struct account user[30];

int clients[30];
int n_of_cc = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void forward(char *buffer, int current) {

	/* extract recipient tokens with strtok
	   
	   if 'token for recipients' length is 0
	   	for each connected device 
		send message
	   else if token > 3
	        compare tokens with usernames
		if match
		send out socket connected with selected username */
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < n_of_cc; i++) {
		if(clients[i] != current) {
			if(send(clients[i], buffer, strlen(buffer), 0) < 0) {
				perror("failed to send message");
				continue;
			
			}
		}
	}
	pthread_mutex_unlock(&mutex);	
}

void *message_interface(void *acc) {
	
	char *buffer;
	struct account current = *((struct account *)acc);

	buffer=(char *)malloc(sizeof(char) * 200);

	while((recv(current.sockno, buffer, 256, 0)) > 0) {
		printf("\n%s\n", buffer);
		forward(buffer, current.sockno);
		memset(buffer, '\0', sizeof(*buffer));	
	}
	pthread_mutex_lock(&mutex);
	printf("user on %s has disconnected\n", current.ip);
	for(int i = 0; i < n_of_cc; i++) {
		if(user[i].sockno == current.sockno) {
			int j = 1;
			while(j < n_of_cc - 1) {
				user[j] = user[j + 1];
				j++;
			}
		}
	}
	n_of_cc--;
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

int sign_in(struct account temp_acc) {
	
	char *message;

	message = (char *)malloc(256);
	
	pthread_mutex_lock(&mutex);
	
	recv(temp_acc.sockno, message, sizeof(temp_acc.username), 0);
	recv(temp_acc.sockno, message, sizeof(temp_acc.username), 0);

	if (strlen(message) > 21) { 
			strcpy(message, "NOTOK");
			send(temp_acc.sockno, message, sizeof(message), 0);
			printf("user entered incorrect username");
			return -1;
	}else {
		strcpy(temp_acc.username, message);
		send(temp_acc.sockno, message, strlen(message), 0);
	}	
	recv(temp_acc.sockno, temp_acc.password, sizeof(temp_acc.password), 0);

	if (strlen(message) > 21) {
		strcpy(message, "NOTOK");
		send(temp_acc.sockno, message, sizeof(message),0);
		return -1;
	}else {
		strcpy(temp_acc.password, message);
	}
	pthread_mutex_unlock(&mutex);
	return 0;
}

int create_acc(struct account new_acc) {

	char *message;
	pthread_t message_interface_t;
	
	message = (char *)malloc(256);
	
	pthread_mutex_lock(&mutex);
	
	recv(new_acc.sockno, message, sizeof(new_acc.username), 0);
		

		if (strlen(message) > 20) { 
				strcpy(message, "NOTOK");
				send(new_acc.sockno, message, sizeof(message), 0);
				printf("user entered incorrect username");
				return -1;
		}else {
			strcpy(new_acc.username, message);
			strcpy(message, "OK");
			send(new_acc.sockno, message, 2, 0);	
		}

		recv(new_acc.sockno, message, sizeof(new_acc.password), 0);

		if (strlen(message) > 20) {
			strcpy(message, "NOTOK");
			send(new_acc.sockno, message, sizeof(message), 0);			
			return -1;
		}else {
			strcpy(new_acc.password, message);
			strcpy(message, "OK");
			send(new_acc.sockno, message, sizeof(message), 0);
			for(int i = 0; i <= MAXCON; i++) {
				if(strcmp(user[i].username, "\0") == 0) {
					i++;
			}else {
				strcpy(user[i].username, new_acc.username);
				strcpy(user[i].password, new_acc.password);
			}
		}
	}	
	pthread_mutex_unlock(&mutex);
	free(message);
	clients[n_of_cc] = new_acc.sockno;
	n_of_cc++;

	pthread_create(&message_interface_t, NULL, (void *)message_interface, (void *)&new_acc);
	pthread_exit(NULL);	

	return 0;	
}

void *handle(void *sock) {
	
	char *message;
	struct account user = *((struct account *)sock);

	message = (char *)malloc(15);
	
	recv(user.sockno, message, 10, 0);

	if (strncmp(message, "sign_in", 7) == 0) {
		while(sign_in(user) == -1) {
			sign_in(user);
		}
	}else if (strncmp(message, "create_acc", 10) == 0) {
		while(create_acc(user) == -1) {
			create_acc(user);
		}
	}
	
	pthread_mutex_unlock(&mutex);

	return 0;

}
int main() { 

	 int sockfd, new_socket; //activity;
	 struct sockaddr_in server_addr, new_addr; 
	 pthread_t recvt;
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

