#include <stdio.h>

#include "socket.h"

int main(void) {
    const char *server_ip = "172.28.0.10";
    int port = 3000;
    connect_to_server(server_ip, port);
    return 0;
}
