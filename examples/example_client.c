#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 7272

int main() { 
	int client_socket;
	struct sockaddr_in server_addr;
	char buffer[256];

	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT);
	server_addr.sin_addr.s_addr=inet_addr("192.168.0.38");

	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("[-] error connecting to server ");
	}
	else {
		printf("[+] successfully connected\n");
		recv(client_socket, buffer, 256, 0);
		printf("[+] Server : %s\n", buffer);

		}

	close(client_socket);

		return 0;
}
	
