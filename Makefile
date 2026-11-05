CC = gcc
# CFLAGS = -Wall -Wextra -O2 -Iinclude
CFLAGS = -Wall -Wextra -g -O0 -Iinclude

all: server client

server: src/server.c src/socket.c src/protocol.c src/dns.c src/utils.c
	$(CC) $(CFLAGS) $^ -o server

client: src/client.c src/socket.c src/protocol.c src/dns.c src/utils.c
	$(CC) $(CFLAGS) $^ -o client

clean:
	rm -f server client