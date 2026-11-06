#include <stdio.h>
#include <unistd.h>
#include <stdio.h>

#include "socket.h"
#include "protocol.h"
#include "utils.h"


int main(void) {
    const char *server_ip = "172.28.0.10";
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
        if ((serialize(buffer, sizeof(buffer), &request)) < 0){
            return -1;
        }

        // send the http request to server via the socket.
        socket_send_all(client_socket, buffer);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}
