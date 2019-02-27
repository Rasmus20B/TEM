#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 7272

void main() { 

	int sockfd;
	struct sockaddr_in server_addr;

	int new_socket;
	struct sockaddr_in new_addr;

	socklen_t addr_size; 
	char message[20];

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

	listen(sockfd, 3);
	addr_size = sizeof(new_addr);

	new_socket = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
	
	fgets(message, 256, stdin);
	send(new_socket, message, strlen(message), 0);	

	
}

