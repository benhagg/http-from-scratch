#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket(), bind(), listen(), accept(), connect()
#include <netinet/in.h> // For struct sockaddr_in
#include <arpa/inet.h>  // For IP address conversion functions (htons, inet_pton)
#include <netdb.h>

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
    if (bind_result < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

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
    if (client_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // CONFIGURE ADDRESS STRUCT
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // convert from little-endian (how it is stored in memory) to big-endian (how it is used in network addressing)
    struct addrinfo *res;
    int status = getaddrinfo(hostname, "8080", NULL, &res); // getaddrinfo abstracts away the sockaddr struct. but i am just using it to pull the IPV4 addr from the docker compose dns and use the sockaddr struct.
    if (status < 0){
        perror("failed to resolve hostname");
        exit(EXIT_FAILURE);
    }
    // Cast res from sockaddr to sockaddr_in, then copy the .sin_addr struct
    struct sockaddr_in *temp_ipv4 = (struct sockaddr_in *)res->ai_addr;
    server_addr.sin_addr = temp_ipv4->sin_addr;

    // CONNECT TO SERVER
    int connect_result = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_result < 0){
        perror("failed to connect to server");
        freeaddrinfo(res);
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    return 1;
}