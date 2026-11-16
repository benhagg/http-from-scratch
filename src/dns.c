#include "dns.h"
#include "http.h"
#include "socket.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int check_ownership(const dns_table *table, const char *ip) {
    if (table == NULL || table->records == NULL || ip == NULL) return 0;
    for (size_t i = 0; i < table->count; i++)
        if (strcmp(table->records[i].ip, ip) == 0) return 0;
    return 1;
}

int register_ip(dns_table *table, const char *hostname, const char *ip) {
    if (table == NULL || table->records == NULL || hostname == NULL || ip == NULL) return -1;
    if (strlen(hostname) >= MAX_DNS_NAME || strlen(ip) >= MAX_IP_STR) return -1;
    if (table->count >= table->capacity || !check_ownership(table, ip)) return -1;
    strcpy(table->records[table->count].hostname, hostname);
    strcpy(table->records[table->count].ip, ip);
    table->count++;
    return 0;
}

const char *lookup(const dns_table *table, const char *hostname) {
    if (table == NULL || table->records == NULL || hostname == NULL) return NULL;
    for (size_t i = 0; i < table->count; i++)
        if (strcmp(table->records[i].hostname, hostname) == 0) return table->records[i].ip;
    return NULL;
}

int dns_table_init(struct dns_table *table, size_t capacity){
    if (table == NULL || capacity == 0) return -1;
    table->records = NULL;
    table->count = 0;
    table->capacity = capacity;
    table->records = malloc(capacity * sizeof *table->records);
    if (table->records == NULL) return -1;
    return 0;
}

// turns 3www7example3com0 into www.example.com
static int decode_name(const unsigned char *buffer, size_t buffer_len,
                       size_t *position, char *hostname) {
    size_t output_length = 0;
    while (*position < buffer_len && buffer[*position] != 0) {
        size_t label_length = buffer[(*position)++];
        if (label_length > 63 || *position + label_length > buffer_len) return -1;
        if (output_length > 0) hostname[output_length++] = '.';
        if (output_length + label_length >= MAX_DNS_NAME) return -1;
        memcpy(hostname + output_length, buffer + *position, label_length);
        output_length += label_length;
        *position += label_length;
    }
    if (*position >= buffer_len || output_length == 0) return -1;
    hostname[output_length] = '\0';
    (*position)++;
    return 0;
}

/*
 * Example custom serialized response:
 * Header:
 *   ID:        0x1234
 *   Questions: 1
 *   Answers:   1
 *
 * Question:
 *   Name: [3]www[6]google[3]com[0]
 *
 * Answer:
 *   Name: [3]www[6]google[3]com[0]
 *   Data: 192.0.2.10
 *
 */
int deserialize_dns(const unsigned char *buffer, size_t buffer_len, DnsRequest *req){
    if (!buffer || !req || buffer_len < 7) return -1;
    // Shift the first byte left 8 bits, then combine it with the second byte. (convert big-endian back to little-endian)
    req->id = (buffer[0] << 8) | buffer[1];
    req->question_count = (buffer[2] << 8) | buffer[3];
    req->answer_count = (buffer[4] << 8) | buffer[5];
    size_t position = 6;
    if (req->question_count != 1 || decode_name(buffer, buffer_len, &position, req->question.name) < 0) return -1;
    req->answer.data[0] = '\0';
    if (req->answer_count == 0) return position == buffer_len ? 0 : -1;
    if (req->answer_count != 1 || decode_name(buffer, buffer_len, &position, req->answer.name) < 0) return -1;
    if (position + 4 != buffer_len) return -1;
    if (!inet_ntop(AF_INET, buffer + position, req->answer.data, sizeof req->answer.data)) return -1;
    return 0;
}

// Writes a 16 bit value into network byte order (big-endian) at the given pointer. This is required becuase the computer stores the value as little-endian
static void write_u16(unsigned char *p, uint16_t value) {
    // unsigned char is a byte-sized integer type that holds values from 0 to 255.
    // p points to where we are going to start writing in the buffer
    p[0] = (unsigned char)(value >> 8);
    p[1] = (unsigned char)(value & 0xff);
}

// turns www.example.com into 3www7example3com0
static size_t encode_name(unsigned char *buffer, size_t buffer_size, const char *hostname) {
    size_t position = 0;
    size_t label_start = 0;
    size_t name_length = strlen(hostname);
    for (size_t i = 0; i <= name_length; i++) {
        if (i != name_length && hostname[i] != '.') continue;
        size_t label_length = i - label_start;
        if (label_length == 0 || label_length > 63 || position + label_length + 1 >= buffer_size) return 0;
        buffer[position++] = (unsigned char)label_length;
        memcpy(buffer + position, hostname + label_start, label_length);
        position += label_length;
        label_start = i + 1;
    }
    if (position >= buffer_size) return 0;
    buffer[position++] = 0; // End of the hostname.
    return position;
}

int serialize_dns(unsigned char *buffer, size_t buffer_size, const DnsRequest *req) {
    if (!buffer || !req || buffer_size < 6 || req->question_count != 1 || req->answer_count > 1) return -1;
    size_t position = 6;
    // real DNS uses big-endian for numeric fields. So we will too (write_u16 writes in big-endian)
    write_u16(buffer, req->id); write_u16(buffer + 2, req->question_count); // write question count
    write_u16(buffer + 4, req->answer_count); // write answer count
    size_t name_size = encode_name(buffer + position, buffer_size - position, req->question.name);
    if (!name_size) return -1;
    position += name_size;
    if (req->answer_count == 0) return (int)position; // if we are not sending an answer, we are done
    unsigned char ip[4]; // reserve 4 bytes space for ipv4 address, unsigned char is 0-255
    if (inet_pton(AF_INET, req->answer.data, ip) != 1) return -1; // ensure that it is a valid ipv4 address
    if (position + 4 > buffer_size) return -1; // if we exceed buffer size
    name_size = encode_name(buffer + position, buffer_size - position - 4, req->answer.name); // we send the name in the answer portion too.
    if (!name_size) return -1;
    position += name_size;
    memcpy(buffer + position, ip, 4); // copy the 4 bytes of the ipv4 address into the buffer
    return (int)(position + 4); // return total number of bytes written to the buffer
}


// clean function to handle all of the dns internals. Take hostname and return ip address. removes the need for a client to handle the serialization and deserialization steps)
int dns_resolve(const char *dns_server_ip, const char *hostname,
                char *ip_buffer, size_t ip_buffer_size) {
    DnsRequest req = {0};
    req.id = 1; req.question_count = 1;
    if (!dns_server_ip || !hostname || !ip_buffer || ip_buffer_size < MAX_IP_STR) return -1;
    if (strlen(hostname) >= sizeof req.question.name) return -1;
    strcpy(req.question.name, hostname);
    unsigned char buffer[512];
    int request_size = serialize_dns(buffer, sizeof buffer, &req);
    if (request_size < 0) return -1;
    int fd = connect_to_server(dns_server_ip, 53);
    if (send(fd, buffer, (size_t)request_size, 0) != request_size) { close(fd); return -1; }
    ssize_t response_size = recv(fd, buffer, sizeof buffer, 0);
    close(fd);
    if (response_size <= 0 || deserialize_dns(buffer, (size_t)response_size, &req) < 0 || req.answer_count != 1) return -1;
    printf("DNS response: ID=%u Questions=%u Answers=%u Name=%s IP=%s\n",
           req.id, req.question_count, req.answer_count,
           req.question.name, req.answer.data);
    fflush(stdout);
    strcpy(ip_buffer, req.answer.data);
    return 0;
}

#ifndef DNS_LIBRARY_ONLY //if DNS_LIBRARY_ONLY is defined, we will not compile the main function for the dns server. This allows us to use the dns functions in other programs without running the dns server code.
int main(void){
    dns_table ip_table = {0};
    if (dns_table_init(&ip_table, 2) < 0) {
        perror("dns table initialization failed");
        return 1;
    }
    register_ip(&ip_table, "server", "172.28.0.10");
    register_ip(&ip_table, "client", "172.28.0.12"); //hardcoded ip for now. we will make dynamic in future. ##FIXME
    int dns_fd = create_server_socket(53); // create socket on port 53 for dns server
    unsigned char buffer[512];
    ssize_t recv_result;
    for(;;){
        int client_fd = accept(dns_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("dns server accept failed");
            return 1;
        }
        while ((recv_result = recv(client_fd, buffer, sizeof buffer, 0)) > 0) {
            DnsRequest req = {0};
            if (deserialize_dns(buffer, (size_t)recv_result, &req) < 0) continue;
            const char *ip = lookup(&ip_table, req.question.name);
            if (ip != NULL) {
                req.answer_count = 1;
                strcpy(req.answer.name, req.question.name);
                strcpy(req.answer.data, ip);
            }
            printf("DNS request: ID=%u Questions=%u Name=%s Answers=%u IP=%s\n",
                   req.id, req.question_count, req.question.name,
                   req.answer_count, req.answer.data);
            fflush(stdout);
            int response_size = serialize_dns(buffer, sizeof buffer, &req);
            if (response_size > 0) send(client_fd, buffer, (size_t)response_size, 0);
        }
        close(client_fd);
    }
}
#endif
