#ifndef SOCKET_H
#define SOCKET_H

// Helper to create and bind a server socket, then start listening
int create_server_socket(int port);

// Helper for a client to connect to a server IP and port
int connect_to_server(const char *ip_address, int port);

#endif