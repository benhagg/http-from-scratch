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
    // 1. Isolate the body first by finding the blank line separator (\r\n\r\n)
    char *body_ptr = strstr(buffer, "\r\n\r\n");
    if (body_ptr != NULL) {
        body_ptr += 2; // Move past the first "\r\n" separator
    }else{
        perror("no request body found");
        return -1;
    }

    // 2. parse the first line of the request which contains the method, path, and version
    char *method_end = strstr(buffer, " ");
    char *path_end = strstr(method_end + 1, " ");
    char *line_end = strstr(buffer, "\r\n");
    if (!line_end || !method_end || !path_end ||
        method_end >= line_end || path_end >= line_end) {
        return -1;
    }
    // terminate all the strings so we can use them as C strings
    *method_end = '\0';
    *path_end = '\0';
    *line_end = '\0';
    // we can now safely assign those strings to the HttpRequest struct becuase they had been terminated above
    req->method = buffer;
    req->path = method_end + 1;
    
    
    char *line_start = line_end + 2;  // start after the request line's CRLF
    size_t content_length = 0; // Initialize content_length to 0
    // 3. iterate over the remaining lines to extract headers
    while(line_start < body_ptr){
        line_end = strstr(line_start, "\r\n");
        if (!line_end) {
            perror("Malformed request: no CRLF found");
            return -1; // Malformed request, no CRLF found
        }
        *line_end = '\0'; // null terminate so we can directly assign the string to the struct
        if (strncmp(line_start, "Host: ", 6) == 0) { // check that first 6 is exactly "Host: "
            char *host = line_start + 6; // Skips "Host: " and sets host equal to the rest of the line (until the \0 delim)
            req->host = host;
        } 
        else if (strncmp(line_start, "Content-Type: ", 14) == 0) {
            char *content_type = line_start + 14;
            req->content_type = content_type;
        }
        else if (strncmp(line_start, "Content-Length: ", 16) == 0) {
            content_length = strtoul(line_start + 16, NULL, 10); //strtoul string to unsigned long (arg 10 means base)
        }
        else if(strncmp(line_start, "Connection: ", 12) == 0) {
            char *connection = line_start + 12;
            req->connection = connection;
        }

        line_start = line_end + 2; // Move to the start of the next line
    }
    // 4. pull the request body
    // we already check that body_ptr is not NULL
    if(content_length != 0){
        // Copy the exact number of bytes specified by Content-Length into req->body
        body_ptr += 2; // Move past the second "\r\n" separator (we move past the first one in step one of this function)
        body_ptr[content_length] = '\0'; // Null-terminate the body string
        
        req->body = body_ptr;
    }

    return 1;
}
