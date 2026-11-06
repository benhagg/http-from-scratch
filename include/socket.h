#ifndef SOCKET_H
#define SOCKET_H

#include <stddef.h>
#include <sys/types.h>

// Helper to create and bind a server socket, then start listening
int create_server_socket(int port);

// Helper for a client to connect to a server IP and port
int connect_to_server(const char *ip_address, int port);

// Sends all bytes in the buffer. Returns 0 on success, -1 on error.
int socket_send_all(int socket_fd, const char *message);

// Reads available bytes. Returns the byte count, 0 on peer close, or -1 on error.
ssize_t socket_receive_bytes(int socket_fd, void *buffer, size_t capacity);

#endif
