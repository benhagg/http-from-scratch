CC = gcc
# CFLAGS = -Wall -Wextra -O2 -Iinclude
CFLAGS = -Wall -Wextra -g -O0 -Iinclude

all: server client dns

server: src/server.c src/socket.c src/http.c src/dns.c src/utils.c
	$(CC) $(CFLAGS) -DDNS_LIBRARY_ONLY $^ -o server

client: src/client.c src/socket.c src/http.c src/dns.c src/utils.c
	$(CC) $(CFLAGS) -DDNS_LIBRARY_ONLY $^ -o client

dns: src/dns.c src/socket.c src/utils.c
	$(CC) $(CFLAGS) $^ -o dns

clean:
	rm -f server client dns
