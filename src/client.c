#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "socket.h"
#include "http.h"
#include "dns.h"
#include "utils.h"


int main(void) {
    const char *dns_server_ip = getenv("DNS_SERVER_IP");
    // try 10x at 1HZ to get the server ip addr from dns.
    int attempt;
    for (attempt = 0; attempt < 10; attempt++) {
        if (dns_register(dns_server_ip, "client") == 0) break;
        puts("could not register with DNS yet; retrying");
        if (attempt < 9) sleep(1);
    }
    if (attempt == 10) {
        fprintf(stderr, "DNS registration timed out\n");
        return 1;
    }
    char server_ip[MAX_IP_STR];
    for (attempt = 0; attempt < 10; attempt++) {
        if (dns_resolve(dns_server_ip, "server", server_ip, sizeof server_ip) == 0)
            break;
        puts("server is not registered yet; retrying DNS lookup");
        if (attempt < 9) sleep(1);
    }
    if (attempt == 10) {
        fprintf(stderr, "DNS lookup timed out\n");
        return 1;
    }
    int port = 3000;
    for(int i=0; i<2; i++){
        int client_socket = connect_to_server(server_ip, port); // this returns the socket
        // build the http request
        HttpRequest request = {0};
        request.method = "GET";
        request.path = "/test_path";
        // create the host by concat ip and port
        char host[32];
        snprintf(host, sizeof(host), "%s:%d", server_ip, port);
        request.host = host;
        request.content_type="text/plain";
        request.body = "test";

        // init the buffer. serialize will fill this buffer.
        char buffer[PROTOCOL_MAX_MESSAGE_SIZE + 1];
        if ((request_serialize(buffer, sizeof(buffer), &request)) < 0){
            return -1;
        }

        // send the http request to server via the socket.
        socket_send_all(client_socket, buffer);
        shutdown(client_socket, SHUT_WR);
        char response_buffer[1024];
        ssize_t response_size;
        while ((response_size = recv(client_socket, response_buffer,
                                     sizeof(response_buffer) - 1, 0)) > 0) {
            response_buffer[response_size] = '\0';
            printf("%s", response_buffer);
        }
        close(client_socket);
        sleep(1);
    }
    return 0;
}
