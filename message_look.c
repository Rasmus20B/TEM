#include <stdio.h>
#include <time.h>
#include <string.h>

int main() {

	char message[256];
	time_t ltime;
	struct tm *info;
	char user[20];
	char buffer[80];
	char time_buffer[80];

//	strcpy(buffer.message, "asdfasdfasdfasdf");
////////buffer.time = time(NULL);
////////strcpy(buffer.user, "jam");
//works fine
//	printf("%s\t%s\t%ld", buffer.user, buffer.message, buffer.time);	
	
	strcpy(message, "asdfasdfasdfasdf");
	
	time( &ltime );

	info = localtime( &ltime );

	strcpy(user, "jam");

	strftime(time_buffer,80,"%x - %T:%M", info);
	
	memset(buffer, '\0', sizeof(buffer)); 
	strcat(buffer, user);
	strcat(buffer, "\t");
	strcat(buffer, message);	
	strcat(buffer, "\t");
	strcat(buffer, time_buffer);

	printf("%s\n", buffer);	

}
