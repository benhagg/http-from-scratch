#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "socket.h"
#include "protocol.h"

int main(void) {
    int port = 3000;

    int server_fd = create_server_socket(port);
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
                deserialize(buffer, &req);
                puts("Deserialized HTTP request body (and the rest of the request too):");
                puts(req.body);
                fflush(stdout);
            }else if (recv_result == 0){
                puts("client closed connection");
            }else{
                perror("recv failed");
            }
        }
    }
    close(server_fd);
    return 0;
}
