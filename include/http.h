#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

// Maximum serialized HTTP request bytes, excluding the C string terminator.
#define PROTOCOL_MAX_MESSAGE_SIZE 1024

//HTTP Request datatype
typedef struct {
    const char *method; // GET, POST etc.
    const char *path; //path to the resource
    const char *host; //hostname or ip and port
    const char *content_type; //type of the body
    const char *connection;
    char *body;
} HttpRequest;

// HTTP response. The body is a string for this simple implementation.
typedef struct {
    int status_code;
    const char *reason_phrase;
    const char *content_type;
    const char *connection;
    char *body;
} HttpResponse;

// Serializes one HTTP request into buffer. Returns 1 on success, -1 on error.
// Example request:
// POST /items HTTP/1.1\r\n
// Host: 172.28.0.10:3000\r\n
// Content-Type: text/plain\r\n
// Content-Length: 4\r\n
// Connection: close\r\n
// \r\n
// test
int request_serialize(char buffer[], size_t buffer_size, const HttpRequest *req);

// Receives one message into buffer and adds a terminating '\0'.
// buffer_size is the total buffer capacity, including space for '\0'.
// Returns 1 when a message is received, 0 if the peer closes before a message,
// or -1 on error (including a message that exceeds the buffer).

int request_deserialize(char *buffer, HttpRequest *req);

// Serializes or parses one HTTP response. Returns 1 on success, -1 on error.
int response_serialize(char buffer[], size_t buffer_size, const HttpResponse *res);
int response_deserialize(char *buffer, HttpResponse *res);

#endif
