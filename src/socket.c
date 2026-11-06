#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket(), bind(), listen(), accept(), connect()
#include <netinet/in.h> // For struct sockaddr_in
#include <arpa/inet.h>  // For IP address conversion functions (htons, inet_pton)

#include "socket.h"
#include <utils.h>


int socket_send_all(int socket_fd, const char *message){
    // send api does not guarentee that we send the whole message so we need to handle the cases it does not.
    int total_len = strlen(message);
    int sent = 0;
    while (sent < total_len){
        // try to send (total_len - send) starting at mem addr (message[0] + sent). retuns num bytes sent.
        ssize_t send_status = send(socket_fd, message + sent, total_len - sent, 0);
        if (send_status <= 0){
            return -1;
        }
        sent = sent + send_status;
    }
    return 0;
}

ssize_t socket_receive_bytes(int socket_fd, void *buffer, size_t capacity){
    return recv(socket_fd, buffer, capacity, 0);
}

int create_server_socket(int port){

    // INIT SOCKET FILE DESCRIPTOR
    // AF_INET is for IPV4, SOCK_STREAM opens a TCP socket, 0 to default to protocol IPPROTO_TCP
    // socket asks the OS to open a socket and pass back a file descriptor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    // CONFIGURE ADDRESS STRUCT
    struct sockaddr_in address;
    address.sin_family = AF_INET; //2 bytes, must match the AF_INET family passed into socket above (IPV4)
    address.sin_addr.s_addr = INADDR_ANY; //4 bytes (0.0.0.0) Listen on all network interfaces
    address.sin_port = htons(port); // 2 bytes, 2^16 or 65536 port addresses
    // there will be a 8 byte buffer here when converted to sockaddr type

    // BIND TO PORT
    // cast address as a sockaddr type, tell bind how many bytes the ipv4 address will be.
    int bind_result = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    handle_error(bind_result, "bind failed");

    // START LISTENING FOR INCOMING CONNECTIONS
    // note: this server is single threaded meaning it can only handle one connection at once. 
    // it must process one connection completely before accepting the next.
    int max_connection_queue = 5; // max pending connections
    int listen_result = listen(server_fd, max_connection_queue);
    if (listen_result < 0){
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int connect_to_server(const char *hostname, int port){
    // INIT SOCKET FILE DESCRIPTOR
    // structurally identical to the one in create_server_socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    handle_error(client_fd, "socket failed");

    // CONFIGURE ADDRESS STRUCT
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // convert from little-endian (how it is stored in memory) to big-endian (how it is used in network addressing)
    int status = inet_pton(AF_INET, hostname, &server_addr.sin_addr);
    if (status != 1) {
        fprintf(stderr, "invalid IPv4 address: %s\n", hostname);
        exit(EXIT_FAILURE);
    }

    // CONNECT TO SERVER
    int connect_result = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_result < 0){
        perror("failed to connect to server");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    // SEND MESSAGE TO SERVER
    socket_send_all(client_fd, "test");
    close(client_fd);

    return 1;
}
