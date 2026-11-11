#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocol.h"
#include "socket.h"
#include "utils.h"

// buffer is the thing we are going to serialize into the correct format bytestring that we can send
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


int deserialize(char *buffer, HttpRequest *req){
    // 1. isolate the first line of the request which contains the method, path, and version
    char *line = strtok(buffer, "\r\n");
    if (line) {
        char *method   = strtok(line, " "); // Resets to parse line
        char *path     = strtok(NULL, " ");
        // char *version  = strtok(NULL, " "); // version unused for now
        req->method = method;
        req->path = path;
    }

    // 2. Isolate the body first by finding the blank line separator (\r\n\r\n)
    // we do this becuase the input string to strtok will modify the original string and we want to keep the body intact for later use.
    char *body_ptr = strstr(buffer, "\r\n\r\n");
    if (body_ptr != NULL) {
        *body_ptr = '\0';          // terminate the header section.
    }else{
        perror("no request body found");
        return -1;
    }

    // 3. iterate over the remaining lines to extract headers
    size_t content_length = 0; // Initialize content_length to NULL
    while ((line = strtok(NULL, "\r\n")) != NULL) {
        if (strncmp(line, "Host: ", 6) == 0) { // check that first 6 is exactly "Host: "
            char *host = line + 6; // Skips "Host: " and sets host equal to the rest of the line (until the \0 delim)
            req->host = strdup(host);
        } 
        else if (strncmp(line, "Content-Type: ", 14) == 0) {
            char *content_type = line + 14;
            req->content_type = strdup(content_type);
        }
        else if (strncmp(line, "Content-Length: ", 16) == 0) {
            content_length = strtoul(line + 16, NULL, 10);
        }
        else if(strncmp(line, "Connection: ", 12) == 0) {
            char *connection = line + 12;
            req->connection = strdup(connection);
        }
    }
    // 4. pull the request body
    // we already check that body_ptr is not NULL
    if(content_length != 0){
        // Copy the exact number of bytes specified by Content-Length into req->body
        char *body_start = body_ptr + 4; // Move past the "\r\n\r\n" separator
        memcpy(req->body, body_start, content_length);

        // Manually ensure it's safely null-terminated for string printing/parsing
        req->body[content_length] = '\0';
    }else{
        perror("Content-Length header not found, cannot extract body");
        return -1;
    }


    return 1;
}
