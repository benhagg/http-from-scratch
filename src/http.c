#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

int request_serialize(char buffer[], size_t buffer_size, const HttpRequest *req){
    if (buffer == NULL || req == NULL || req->method == NULL ||
        req->host == NULL || req->path == NULL) {
        return -1;
    }

    if (strcmp(req->method, "GET") != 0 &&
        strcmp(req->method, "POST") != 0 &&
        strcmp(req->method, "PUT") != 0 &&
        strcmp(req->method, "DELETE") != 0) {
        return -1;
    }

    const char *body = req->body != NULL ? req->body : "";
    const char *content_type = req->content_type != NULL
        ? req->content_type
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
        req->method,
        req->path,
        req->host,
        content_type,
        body_length,
        body);

    if (message_length < 0 || (size_t)message_length >= buffer_size) {
        return -1;
    }
    return 1;
}

int request_deserialize(char *buffer, HttpRequest *request){
    if (buffer == NULL || request == NULL) {
        return -1;
    }

    char *headers_end = strstr(buffer, "\r\n\r\n");
    char *line_end = strstr(buffer, "\r\n");
    char *method_end = strchr(buffer, ' ');
    if (headers_end == NULL || line_end == NULL || method_end == NULL ||
        method_end >= line_end) {
        return -1;
    }

    char *path_end = strchr(method_end + 1, ' ');
    if (path_end == NULL || path_end >= line_end) {
        return -1;
    }

    *method_end = '\0';
    *path_end = '\0';
    *line_end = '\0';
    request->method = buffer;
    request->path = method_end + 1;
    request->host = NULL;
    request->content_type = NULL;
    request->connection = NULL;

    size_t content_length = 0;
    char *line_start = line_end + 2;
    while (line_start < headers_end) {
        line_end = strstr(line_start, "\r\n");
        if (line_end == NULL || line_end > headers_end) {
            return -1;
        }
        *line_end = '\0';

        if (strncmp(line_start, "Host: ", 6) == 0) {
            request->host = line_start + 6;
        } else if (strncmp(line_start, "Content-Type: ", 14) == 0) {
            request->content_type = line_start + 14;
        } else if (strncmp(line_start, "Content-Length: ", 16) == 0) {
            content_length = strtoul(line_start + 16, NULL, 10);
        } else if (strncmp(line_start, "Connection: ", 12) == 0) {
            request->connection = line_start + 12;
        }
        line_start = line_end + 2;
    }

    request->body = headers_end + 4;
    if (strlen(request->body) < content_length) {
        return -1;
    }
    request->body[content_length] = '\0';
    return 1;
}

int response_serialize(char buffer[], size_t buffer_size, const HttpResponse *res){
    if (buffer == NULL || res == NULL || res->status_code < 100 ||
        res->status_code > 599 || res->reason_phrase == NULL) {
        return -1;
    }

    const char *content_type = res->content_type != NULL
        ? res->content_type
        : "text/plain";
    const char *connection = res->connection != NULL
        ? res->connection
        : "close";
    const char *body = res->body != NULL ? res->body : "";
    size_t body_length = strlen(body);

    int message_length = snprintf(
        buffer,
        buffer_size,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: %s\r\n"
        "\r\n"
        "%s",
        res->status_code,
        res->reason_phrase,
        content_type,
        body_length,
        connection,
        body);

    if (message_length < 0 || (size_t)message_length >= buffer_size) {
        return -1;
    }
    return 1;
}

int response_deserialize(char *buffer, HttpResponse *res){
    if (buffer == NULL || res == NULL ||
        strncmp(buffer, "HTTP/1.1 ", 9) != 0) {
        return -1;
    }

    char *line_end = strstr(buffer, "\r\n");
    char *headers_end = strstr(buffer, "\r\n\r\n");
    char *first_space = strchr(buffer, ' ');
    if (line_end == NULL || headers_end == NULL || first_space == NULL ||
        first_space >= line_end) {
        return -1;
    }

    char *second_space = strchr(first_space + 1, ' ');
    if (second_space == NULL || second_space >= line_end) {
        return -1;
    }

    char *status_end = NULL;
    long status_code = strtol(first_space + 1, &status_end, 10);
    if (status_end != second_space || status_code < 100 || status_code > 599) {
        return -1;
    }

    *second_space = '\0';
    res->status_code = (int)status_code;
    res->reason_phrase = second_space + 1;
    *line_end = '\0';
    res->content_type = NULL;
    res->connection = NULL;
    res->body = headers_end + 4;

    size_t content_length = 0;
    char *line_start = line_end + 2;
    while (line_start < headers_end) {
        line_end = strstr(line_start, "\r\n");
        if (line_end == NULL || line_end > headers_end) {
            return -1;
        }
        *line_end = '\0';

        if (strncmp(line_start, "Content-Type: ", 14) == 0) {
            res->content_type = line_start + 14;
        } else if (strncmp(line_start, "Content-Length: ", 16) == 0) {
            content_length = strtoul(line_start + 16, NULL, 10);
        } else if (strncmp(line_start, "Connection: ", 12) == 0) {
            res->connection = line_start + 12;
        }
        line_start = line_end + 2;
    }

    if (strlen(res->body) < content_length) {
        return -1;
    }
    res->body[content_length] = '\0';
    return 1;
}
