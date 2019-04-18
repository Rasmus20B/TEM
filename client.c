#include <stdio.h>	//used for standard library functionality including memory management
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h> // used for networking functionality
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>	//used when assembling messages

#include <openssl/crypto.h>
#include <openssl/ssl.h>	//crypto functionality
#include <openssl/err.h>

//arbitrary port number
#define PORT 7272

//colours for formatting
#define ANSI_COLOR_RED	"\x1b[31m"	
#define ANSI_COLOR_GREEN "\x1b[32m"


/* This program is the client that users interact with */
/* connects to server */

struct account {
	int cli_fd;
	char username[20];
	char password[20];	
	
};

char save;	//used to determine whether user wants to save messages to text file

typedef pthread_t OT_THREAD_HANDLE;

SSL_CTX* initCTX() {

	/* function used to initialize SSL context */

	const SSL_METHOD *meth;
	SSL_CTX *ctx;
	
	//load cryptography algorithms 
	OpenSSL_add_all_algorithms();
	//load error messages for printing in case of errors
	SSL_load_error_strings();
	//
	meth = TLS_method();
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}

void show_certs(SSL *ssl) {

	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if(cert != NULL) {
		printf("server certificates:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("subject: %s\n", line);
		free(line);
		X509_free(cert);
	}
	else {
		printf("no certificate\n");
	}
}

int save_to_file(char *buffer) {

	/* this function takes the buffer either from received messages or sent messages and saves them to a file "tem_log.tem" */

	//save the buffer pointer to str	
	char *str = buffer; 	
	//pointer to log file
	FILE *fp;
			
	//opening log file to append	
	fp = fopen("tem_log.tem", "a");

	//check if pointer is null
	if(fp == NULL) {
		//return error to user and exit
		perror("pointer error ");
		exit(1);
	}
	
	//write the buffer to the file fp points to
	fprintf(fp, "%s", str);

	//close the file that fp points to
	fclose(fp);
	
	return 0;
}

void *receive(void *acc) {

	/* a new thread to enable multiplexing between receiving and sending messages */

	char *buffer;
	struct account *user;
	user = (struct account *) acc;
	int len;
	
	//allocate appropriate memory to buffer
	buffer = (char *)malloc(256);	

	//while the length of the message received from server is more than 0
	while((len = recv(user->cli_fd, buffer, 256, 0)) > 0) {	
		//output buffer to standard output
		fputs(buffer, stdout);
		//reset buffer to null
		memset(buffer, '\0', sizeof(&buffer));
		//print a newline for formatting purposes 
		printf("\n");
		}		
	//exit recieve thread
	pthread_exit(NULL);
}

int message_interface(struct account acc) {

	/* function to allow user to enter input and send it to server */

	char *buffer;
	char *contents;
	time_t ltime;
	struct tm *info;
	char time_buffer[20];
	char *newline;
	pthread_t receive_t;
	
	//allocate appropriate amount of memory to both buffer and contents
	buffer = (char *)malloc(256);
	contents = (char *)malloc(200);
	
	//if the thread is not successfully created
	if(pthread_create(&receive_t, NULL, receive, (void *)&acc) != 0) {
		//print error message
		perror("failed to create thread ");
	}
	//if the thread is successfully created
	else {
		printf("would you like to save messages to a local file?\t");
		//receives input from user specifying whether or not they want to save messages
		fgets(&save, 3, stdin);
		
		//set newline to the first occurance of \n in acc.save
		newline = strchr(&save, '\n');
		//sets the value that newline points to to \0
		*newline = '\0';
	
		printf("Enter a message... : ");
		
		//while the user enters messages
		while(fgets(contents, 202, stdin) > 0) {	
			
			//set newline to the first occurance of \n in contents
			newline = strchr(contents, '\n');
			//set the value that newline points to to \0
			*newline = '\0';
			
			//call time function with appropriate variable
			time(&ltime);
			
			//set info to system time
			info = localtime(&ltime);
			
			//format info into string time_buffer
			strftime(time_buffer, 20, "%x - %T:%M", info);
			
			//assemble each piece of the message into a complete buffer
			strcat(buffer, acc.username);
			strcat(buffer, " : \t");
			strcat(buffer, contents);
			strcat(buffer, "\t\t\t");
			strcat(buffer, time_buffer);
			strcat(buffer, "\n");
			
			//send assembled message to server
			send(acc.cli_fd, buffer, strlen(buffer), 0);
			
			//print the message to local user
			printf(ANSI_COLOR_GREEN "%s", buffer);	
			
			//if the user has chosen to save messages
			if(save == 'y') {
				//call save_to_file function
				save_to_file(buffer);
			}
			
			//reset buffer
			memset(buffer, '\0', strlen(buffer));

			}
		}
	//wait for completion of recieve thread when finished
	pthread_join(receive_t, NULL);	
	//close socket
	close(acc.cli_fd);

	return 0;	
}
	
int create_acc(int client_socket, SSL *ssl) {

	/* sends new user credentials to server to be saved for sign in on return uses */
	
	char *buffer;
	struct account *new_acc;
	char *newline;
	
	
	//allocate memory for buffer and account structure 'new_acc'
	new_acc = (struct account*)malloc(sizeof(struct account));		
	buffer = (char*)calloc(sizeof(char), sizeof(char)*256);

	puts("please enter your username\n"); 

	//receives username from user
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin);
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin);
	
	//converts trailing newline left in by fgets to nullbyte
	newline = strchr(new_acc->username, '\n');
	*newline = '\0';

	//sends username to server to verify
	SSL_write(ssl, new_acc->username, strlen(new_acc->username));	
	//receives reply
	SSL_read(ssl, buffer, sizeof(buffer));

	//if reply is NOTOK then 
	if (strncmp(buffer, "NOTOK", 5) == 0) {
		puts("invalid username\n");
		//return error value
		return -1;
	}
	//if reply is OK
	else if (strncmp(buffer, "OK", 2) == 0) {
	
		puts("please enter your password\n");
		//receive password from user
		fgets(new_acc->password, 22, stdin);	
		//sends password to server to verify
		SSL_write(ssl, new_acc->password, strlen(new_acc->password));
		//recieves reply
		SSL_read(ssl, buffer, sizeof(buffer));

		//if reply == NOTOK then 
		if (strncmp(buffer, "NOTOK", 5) == 0) {
			printf("incorrect password");
			//frees memory allocated to password inside structure
			free(new_acc->password);
			return -1;
		//if reply == OK then
		}else {
			//free memory and call messaging interface
			free(buffer);

			//gets new_acc ready to be passed into message interface
			new_acc->cli_fd = client_socket;

			//calls message interface
			message_interface(*new_acc);
		}
		
	}	
	return 0;
}

int sign_in(int client_socket, SSL *ssl) { 

	/* allows user to enter existing credentials in order to use a known username */
		
	char *buffer;
	struct account *new_acc;
	char *newline;

	//allocates memory to new_acc structure
	new_acc = (struct account*)malloc(sizeof(struct account));	
	//allocates memory to buffer
	buffer = (char *)calloc(sizeof(char), sizeof(char)*256);	

	puts("please enter you username\n");
	
	//recieves user input
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin);	
	//repeat function to fix function hanging
	fgets(new_acc->username, (sizeof(new_acc->username)+2), stdin); 
	
	//set newline to the first occurance of a newline in username
	newline = strchr(new_acc->username, '\n');
	//set the value pointed to by newline to \0
	*newline = '\0';


	//sends username to server to verify
	SSL_write(ssl, new_acc->username, strlen(new_acc->username));	
	//receives reply
	SSL_read(ssl, buffer, sizeof(buffer));

	//if reply is NOTOK then 
	if (strncmp(buffer, "NOTOK", 5) == 0) {
		puts("invalid username\n");
		//free memory allocated to buffer
		free(buffer);
		//returns error value
		return -1;
	}
	//if reply is OK then
	else if (strncmp(buffer, "OK", 2) == 0) {
	
	puts("please enter your password\n");

	//receives password 
	fgets(new_acc->password, sizeof(new_acc->password), stdin);
	
	//set newline to the first occurance of a newline in password
	newline = strchr(new_acc->password, '\n');
	//set the value pointed to by newline to \0
	*newline = '\0';

	//sends password to server to verify
	SSL_write(ssl, new_acc->password, strlen(new_acc->password));
	//recieves reply
	SSL_read(ssl, buffer, sizeof(buffer));

	//if reply == NOTOK then 
	if (strncmp(buffer, "NOTOK", 5) == 0) {
		printf("incorrect password");	
		//free memory allocated to buffer
		free(buffer);
		//return error value
		return -1;
	}else {	
		//free memory allocated to buffer
		free(buffer);
		//get new_acc structure ready to pass to message interface
		new_acc->cli_fd = client_socket;
		
		//call message interface function
		message_interface(*new_acc);
		
		}
	}
	return 0;
	
}

int server_connect(char dest_ip_addr[14]) { 

	/* function to connect the client to the specified server */
	
	int client_socket;
	char *buffer;	
	int choice = 0;	

	SSL_CTX *ctx;
	SSL *ssl;
	
	
	//initialises a 'sockaddr_in' structure called 'server_addr'
	struct sockaddr_in server_addr;

	//allocates appropriate amount of memory for buffer
	buffer = (char *)malloc(20);	

	ctx = initCTX();

	//creates the TCP client socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);	
	//overwrites any memory stored in 'server_addr' to null
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
		//return error to main function
		return  -1;
	}

	printf("[+] successfully connected\n");

	ssl = SSL_new(ctx);

	SSL_set_fd(ssl, client_socket);

	if(SSL_connect(ssl) == -1) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	printf("using %s encryption\n", SSL_get_cipher(ssl));
	show_certs(ssl);

	//print menu
	printf("sign in : 1\ncreate account : 2\njoin as guest : 3\n");
		
	//call sign_in function when 1 is selected, call create_account when 2 is selected, else loop
	while (choice == 0) {
		//get choice from user for use in menu
		scanf(" %d", &choice);
		if (choice == 1) {
			//copies message that will notify server of sign in
			strcpy(buffer, "sign_in123");
			//sends server message to initiate sign in process server-side
			SSL_write(ssl , buffer, 0);
			//calls sign in function 
			sign_in(client_socket, ssl); 	
		}else if (choice == 2) {
			//copies message that will notify server of create account
			strcpy(buffer, "create_acc");
			printf("%s\n", buffer);
			//sends server mesage to initiate the creation of a new account	
			SSL_write(ssl, buffer, strlen(buffer));	
			//calls create account function
			create_acc(client_socket, ssl);	
		}else {
			//notifies user to enter a different choice
			puts("please enter a valid choice");	
			//resets the value of choice to ensure the loop continues
			choice = 0;	
		}
	}
	//frees memory taken up by buffer variable
	free(buffer);	
	return 0;

}

int main(int argc, char** argv) {

	/* main function that takes address of a server to connect to */

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
		strncpy(dest_ip_addr, argv[1], 15);
	}
	
	//while connect function fails
	while (cor_ip == -1) {	
		// if user has not entered valid IP address from command line
		if (dest_ip_addr == NULL || strlen(dest_ip_addr) < 7 || strlen(dest_ip_addr) > 15) { 		
			puts("please enter the address of the server you would like to join: ");
		
			// receives server IP address from standard input
			fgets(dest_ip_addr, 15, stdin);		
			//stores the return value of 'connect to server' in cor_ip so it can be reviewed before continuing
			cor_ip = server_connect(dest_ip_addr);	
			}else {	// if user has entered IP address when opening program 	

			//stores the return value of 'server_connect' in cor_ip so it can be reviewed before continuing
			cor_ip = server_connect(dest_ip_addr);		
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
	
