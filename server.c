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

#include <openssl/ssl.h>
#include <openssl/err.h>


#define PORT 7272
#define PENDMAX 3
#define MAXCON 30

struct account {
	char username[20];
	char password[20];
	char ip[INET_ADDRSTRLEN];
	int sockno;	
};

struct account user[30];

	int clients[30];
	int n_of_cc = 0;

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



SSL_CTX *init_serverCTX() {

	SSL_METHOD *meth;
	SSL_CTX *ctx;

	OpenSSL_add_all_algorithms();

	SSL_load_error_strings();

	meth = TLS_method();
	
	ctx = SSL_CTX_new(meth);

	if(ctx == NULL) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}

void load_certificates(SSL_CTX *ctx, char *cert_file, char *key_file) {

	if(SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		abort();
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		abort();
	}	
}

void show_certs(SSL *ssl) {
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if(cert != NULL) {
		printf("server certificates:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Server: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Client: %s\n", line);
		free(line);
		X509_free(cert);
	}else {
		printf("no certificates\n");
	}
}		

void forward(char *buffer, int current) {

	/* extract recipient tokens with strtok
	   
	   if 'token for recipients' length is 0
	   	for each connected device 
		send message
	   else if token > 3
	        compare tokens with usernames
		if match
		send out socket connected with selected username */
	
	//lock thread
	pthread_mutex_lock(&mutex);
	//loop through array of conected sockets
	for(int i = 0; i < n_of_cc; i++) {
		//when the socket number isn't the same as the current connected client, forward message
		if(clients[i] != current) {
			if(send(clients[i], buffer, strlen(buffer), 0) < 0) {
				//print error message if send fails
				perror("failed to send message");
				continue;
			
			}
		}
	}
	//unlock thread
	pthread_mutex_unlock(&mutex);	
}

void *message_interface(void *acc) {
	
	char *buffer;
	//reference pointer to structure acc so it can be used in function
	struct account current = *((struct account *)acc);
	
	//allocate memory to buffer
	buffer=(char *)malloc(256);
	
	//while thread receives message without error
	while((recv(current.sockno, buffer, 256, 0)) > 0) {
		//print buffer to screen
		printf("\n%s\n", buffer);
		//forward to other clients
		forward(buffer, current.sockno);
		//reset buffer
		memset(buffer, '\0', sizeof(*buffer));	
	}

	//lock thread as global variable is going to be altered
	pthread_mutex_lock(&mutex);
	//notify server when a user disonnects
	printf("user on %s has disconnected\n", current.ip);
	
	//decrement number of connected clients (array of socket numbers)
	n_of_cc--;

	//unlock thread
	pthread_mutex_unlock(&mutex);
	//exit thread
	pthread_exit(NULL);
}

int sign_in(struct account temp_acc, SSL *ssl) {

	/* function for signing in to existing account */
	
	char *message;
	int i;
	pthread_t message_interface_t;
	
	//allocate memory for message
	message = (char *)malloc(256);	
	
	//receive username from client
	recv(temp_acc.sockno, message, sizeof(temp_acc.username), 0);	
	
	/* loop through array of records to check if username is already stored */
	for(i = 0; i <= MAXCON; i++) {
		//compare message to record username
		if(strncmp(user[i].username, message, strlen(message)) == 0) {
		//if message and stored username are the same, send OK to client
		strcpy(message, "OK");
		send(temp_acc.sockno, message, sizeof(message), 0);
		printf("user has entered %s in username field\n", user[i].username);
		//exit loop
		break;
		}
		//if loop reaches end of array 
		else if(i == MAXCON) {	
			printf("user entered non-existent username\n");
			//send NOTOK to user
			strcpy(message, "NOTOK");
			send(temp_acc.sockno, message, sizeof(message), 0);	
			//return error value
			return -1;
		}
	}
	//receive password from client
	recv(temp_acc.sockno, message, sizeof(temp_acc.password), 0);
	
	//use i from previous loop to check if message is same as record's password
	if(strncmp(user[i].password, message, strlen(message)) == 0) {
		//if there is a match, send OK to client
		strcpy(message, "OK");
		send(temp_acc.sockno, message, sizeof(message), 0);
	}else {
		//if there is no match, send NOTOK to client
		strcpy(message, "NOTOK");
		send(temp_acc.sockno, message, sizeof(message), 0);	
		//return error value
		return -1;
	}
	//free memory allocated to message	
	free(message);
	//create new thread for messaging interface and pass temp account details to it 
	pthread_create(&message_interface_t, NULL, (void *)message_interface, (void *)&temp_acc);
	//exit handle 
	pthread_exit(NULL);

	return 0;
}

int create_acc(struct account new_acc, SSL *ssl) {

	/* function for account creation */

	char *message;
	pthread_t message_interface_t;
	
	//allocate space for message
	message = (char *)malloc(30);
	
	//lock thread so user[i] is not edited by multiple clients
	pthread_mutex_lock(&mutex);
	
	//receive temporary username 
	SSL_read(ssl, message, sizeof(new_acc.username));
		
		//if username is too long then send back error message to client
		if (strlen(message) > 20) { 
				strcpy(message, "NOTOK");
				SSL_write(ssl, message, sizeof(message));
				//unlock thread so other clients on other threads can enter function
				pthread_mutex_unlock(&mutex);
				//return error value value to calling function
				return -1;
		}else {
			//copies message received from client into new_acc structure
			strcpy(new_acc.username, message);
			//copy OK into message to send to client
			strcpy(message, "OK");
			SSL_write(ssl, message, 2);	
		}
		
		//receieve password from client
		SSL_read(ssl, message, sizeof(new_acc.password));
		
		//if password is too long
		if (strlen(message) > 20) {
			//copy NOTOK to message to send to client
			strcpy(message, "NOTOK");
			SSL_write(ssl, message, sizeof(message));			
			//unlock thread to allow other clients to enter threads after function error
			pthread_mutex_unlock(&mutex);
			//return error value
			return -1;
		}else {
			//copy user message into new_acc structure
			strcpy(new_acc.password, message);
			//copy OK into message to send to client
			strcpy(message, "OK");
			SSL_write(ssl, message, sizeof(message));

			/* interative loop through central array of records */

			//checks each username in array of records for NULL
			for(int i = 0; i <= MAXCON; i++) {
				//if the first character of the username is not NULL (already been written to)
				if(user[i].username[0] != '\0') {
					//increment i to move to next record	
					i++;
			}else {
				//if the username is null, then copy the contents of new_acc to central array of records
				strcpy(user[i].username, new_acc.username);
				strcpy(user[i].password, new_acc.password);
			}
		}
	}	
	//unlock thread because it is finished editing array of records
	pthread_mutex_unlock(&mutex);
	//free memory allocated to message
	free(message);
	//copy the sock.no to array of socket numbers (connected clients)
	clients[n_of_cc] = new_acc.sockno;
	//increment array of socket numbers
	n_of_cc++;
	
	//create thread for messaging interface and pass account structure to it
	pthread_create(&message_interface_t, NULL, (void *)message_interface, (void *)&new_acc);
	//exit handler
	pthread_exit(NULL);	

	return 0;	
}

void *handle(void *sock) {

	/* main menu system for sign in and account creation*/
	
	char *message;
	

	SSL *ssl;
	SSL_CTX *ctx;
	

	struct account user = *((struct account *)sock);

	ctx = init_serverCTX();

	load_certificates(ctx, "cert.pem", "privkey.pem");
	
	//allocate appropriate memory for message
	message = (char *)malloc(256);
	
	
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, user.sockno);

	if(SSL_accept(ssl) == -1) {
		ERR_print_errors_fp(stderr);
	}
	else {
		show_certs(ssl);
		*message = SSL_read(ssl, message, sizeof(message));
	}


	//receive message from client to navigate menu	
	if (strncmp(message, "sign_in123", 10) == 0) {
		//free allocated memory to message as it is not used again
		free(message);
		//loop sign in function while it returns error
		while(sign_in(user, ssl) == -1) {
			sign_in(user, ssl);
		}
	}else if (strncmp(message, "create_acc", 10) == 0) {
		//free allocated memory to message as it is not used again
		free(message);
		//loop create account function while it returns error
		while(create_acc(user, ssl) == -1) {
			create_acc(user, ssl);
		}
	}
	
	pthread_mutex_unlock(&mutex);

	return 0;

}
int main() { 

	/* main function that accepts incoming connections from clients and creates a new thread to keep track of each one */

	 int sockfd, new_socket; //activity;
	 struct sockaddr_in server_addr, new_addr; 
	 pthread_t handle_t;
	 struct account user;
 	 socklen_t addr_size;  
	 
	
	 //clear the terminal
	 system("clear");

	 

	 
	
	 //create socket and store in sockfd
	 sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	 //set address information structure to null
	 memset(&server_addr, '\0', sizeof(server_addr));
	
	 //if return value of socket() call is an error value print error
	 if (sockfd == -1) {
		 perror("[-] error opening socket ");
	 }
	 else {
		 printf("[+] socket successfully opened\n");
	 }
	
	 //set address information 
	 //set address family to IPv4
	 server_addr.sin_family=AF_INET;
	 //set port to predefined PORT
	 server_addr.sin_port=htons(PORT);
	 //set address to addresses pointing to local machine
	 server_addr.sin_addr.s_addr= INADDR_ANY;
	
	 //bind socket to port 
	 if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		 //print error if bind() call returns error value
		 perror("[-] error binding socket to address");
	 }
	 else { 
		 puts("[+] socket successfully bound to port");
	 }
	
	 //listen on socket for incoming connections
	 if (listen(sockfd, PENDMAX) < 0) {
		 perror("[-] error listening ");
	 }
	 else {
 		 printf("[+] listening on port %i...\n", PORT);
	 }
			
	 /*monitoring multiple clients */
	
	 //infinite loop
	 while(1) {
		 //generate a new socket for each accepted connection
		 new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
		 //exit program if return value for accept() call is error value
		 if(new_socket < 0) {
			 exit(1);
		 }
		 pthread_mutex_lock(&mutex);	 
		 //notify server when a user connects 
		 printf("user has connected on %s\n", inet_ntoa(new_addr.sin_addr));
		 //add new socket to socket user structure
		 user.sockno = new_socket;
		 //copy ip address to user structure 
		 strcpy(user.ip, inet_ntoa(new_addr.sin_addr));
		 //create new threat to handle multiple clients interacting with menu
		 pthread_create(&handle_t, NULL, handle, &user);
		 pthread_mutex_unlock(&mutex);
		 
	 }
	 return 0;
}

