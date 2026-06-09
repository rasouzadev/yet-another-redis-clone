CC = gcc
CFLAGS = -Wall -Wextra -O2

all: server client

server: server.c utils.c utils.h
	$(CC) $(CFLAGS) server.c utils.c -o server

client: client.c utils.c utils.h
	$(CC) $(CFLAGS) client.c utils.c -o client

clean:
	rm -f server client
