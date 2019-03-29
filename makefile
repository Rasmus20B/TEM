CC=gcc
CFLAGS_INC := -lpthread -lcrypto
CFLAGS := -g -Wall $(CFLAGS_INC)

all: client server

client: client.c
	$(CC) -o tem client.c $(CFLAGS)

server: server.c
	$(CC) -o tem_server server.c $(CFLAGS)
