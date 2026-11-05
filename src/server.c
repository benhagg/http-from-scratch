#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "socket.h"

int main(void) {
    int port = 3000;

    int server_fd = create_server_socket(port);
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        return 1;
    }

    puts("accepted client connection");
    close(client_fd);
    close(server_fd);
    return 0;
}
