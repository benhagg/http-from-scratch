#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "socket.h"
#include "http.h"
#include "dns.h"

int main(void) {
    int port = 3000;
    int server_fd = create_server_socket(port);
    const char *dns_server_ip = getenv("DNS_SERVER_IP");
    int attempt;
    for (attempt = 0; attempt < 10; attempt++) {
        if (dns_register(dns_server_ip, "server") == 0) break;
        puts("could not register with DNS yet; retrying");
        if (attempt < 9) sleep(1);
    }
    if (attempt == 10) {
        fprintf(stderr, "DNS registration timed out\n");
        close(server_fd);
        return 1;
    }

    char buffer[1024]; // we can recieve up to 1024 bytes
    int recv_result;
    for (;;){
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            close(server_fd);
            return 1;
        }

        while((recv_result = recv(client_fd, buffer, sizeof(buffer), 0)) > 0){
            if(recv_result > 0){
                buffer[recv_result] = '\0';
                HttpRequest req = {0};
                request_deserialize(buffer, &req);
                printf("HTTP body: %s\n", req.body);
                fflush(stdout);
            }else if (recv_result == 0){
                puts("client closed connection");
            }else{
                perror("recv failed");
            }
        }

        HttpResponse response = {0};
        response.status_code = 200;
        response.reason_phrase = "OK";
        response.content_type = "text/plain";
        response.body = "this is a response from the server";
        char response_buffer[1024];
        if (response_serialize(response_buffer, sizeof(response_buffer), &response) < 0 ||
            socket_send_all(client_fd, response_buffer) < 0) {
            perror("failed to send HTTP response");
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}
