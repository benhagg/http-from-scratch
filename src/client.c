#include <stdio.h>
#include <unistd.h>
#include <stdio.h>

#include "socket.h"

int main(void) {
    const char *server_ip = "172.28.0.10";
    int port = 3000;
    for(int i=0; i<10; i++){
        connect_to_server(server_ip, port);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}
