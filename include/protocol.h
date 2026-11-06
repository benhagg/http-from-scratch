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
    const char *body;
} HttpRequest;

// Serializes one HTTP/1.1 request into buffer. Returns byte count, or -1 on error.
// Example request:
// POST /items HTTP/1.1\r\n
// Host: 172.28.0.10:3000\r\n
// Content-Type: text/plain\r\n
// Content-Length: 4\r\n
// Connection: close\r\n
// \r\n
// test
int serialize(char buffer[], size_t buffer_size, const HttpRequest *request);

// Receives one message into buffer and adds a terminating '\0'.
// buffer_size is the total buffer capacity, including space for '\0'.
// Returns 1 when a message is received, 0 if the peer closes before a message,
// or -1 on error (including a message that exceeds the buffer).

// int deserialize(const HttpRequest *request);

#endif
