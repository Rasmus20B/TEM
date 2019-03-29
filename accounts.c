#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct account {
	char password[20];
	char username[20];
};

struct account a[5];


void print_records(int n) {

	for(int i = 0; i <= n - 1; i++) {
		printf("username %s\n", a[i].username);
	}
	
}

int main() {
		
	int i = 0;
	char *choice = "";
	int number_of_records = 0;
	
	choice = (char *)malloc(sizeof(char));	

	while(i <= 5) {
		puts("would you like to enter a new username(1) or remove a username(2)?");
		scanf(" %s", choice);
		if(strcmp(choice, "Y")== 0){
			puts("please enter a username");
			fgets(choice, 3, stdin);
			fgets(a[i].username, 22, stdin);
			number_of_records++;	
			i++;
		}else{
		       break;
		}	     
	}
	print_records(number_of_records);	

	return 0;
	
}


