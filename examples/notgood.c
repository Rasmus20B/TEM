#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#define PORT 7272
#define TRUE 1
#define FALSE 0


int initialise_socket() {

	int opt = 1;
	int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, sd, max_sd, valread;
	struct sockaddr_in server_addr;
	char *message[256];

	fd_set readfds;

	if( (master_socket = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed to open ");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, '\0', sizeof(server_addr));

	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port=htons(PORT);

	if (bind(master_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen (master_socket, 3) < 0) {
		perror("listen ");
	}

	addrlen = sizeof(server_addr);
	puts("waiting for connections");

	while(TRUE) {
		FD_ZERO(&readfds);

		FD_SET(master_socket, &readfds);
		int max_sd = master_socket;

		for (int i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (sd > 0)
				FD_SET(sd, &readfds);


			if (sd > max_sd)
				max_sd = sd;
		}

		activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno!=EINTR)) {
			printf("select error");
		}

		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *)&server_addr, (socklen_t*)&addrlen)) < 0) {
				perror("accept ");
				exit(EXIT_FAILURE);
			}

			printf("new connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
			if (send(new_socket, *message, strlen(*message), 0) != strlen(*message) ) {
				perror("send ");
			}

			puts("welcome message szend sucessfully");

			for (int i = 0; i < max_clients; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n", i);

					break;
				}

		}
	}

		for (int i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				if ((valread = read(sd, *message, sizeof(*message))) == 0) {
					getpeername(sd, (struct sockaddr*)&server_addr, (socklen_t*)&addrlen);
					printf("host disconnected, ip %s, port %d \n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
					close(sd);
					client_socket[i] = 0;
				}
				else {
					*message[valread] = '\0';
					send(sd, *message, strlen(*message), 0);
				}
			}
		}
	}

}

int main() {
	
	initialise_socket();

	return 0;

}


