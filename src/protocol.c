#include <string.h>
#include <stdio.h>

#include "protocol.h"
#include "socket.h"
#include "utils.h"

int serialize(char buffer[], size_t buffer_size, const HttpRequest *request){
    if (request == NULL || request->method == NULL ||
        request->host == NULL || request->path == NULL) {
        perror("something is null that should not be in http request");
        return -1;
    }

    // method validation
    if (strcmp(request->method, "GET") != 0 &&
        strcmp(request->method, "POST") != 0 &&
        strcmp(request->method, "PUT") != 0 &&
        strcmp(request->method, "DELETE") != 0) {
        perror("invalid request method");
        return -1;
    }

    const char *body = request->body != NULL ? request->body : "";
    // default to text/plain
    const char *content_type = request->content_type != NULL
        ? request->content_type
        : "text/plain";
    size_t body_length = strlen(body);

    int message_length = snprintf(
        buffer,
        buffer_size,
        "%s %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        request->method,
        request->path,
        request->host,
        content_type,
        body_length,
        body);

    if (message_length < 0 || (size_t)message_length >= buffer_size) {
        perror("message length exceeded buffer or snprintf failed to serialize");
        return -1;
    }

    // send the serialized http request over the socket
    return 1;
}


// int deserialize(const HttpRequest *request){
//     // will implement later
//     return 1
// }
