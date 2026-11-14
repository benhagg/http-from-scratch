// there is other metadata in the dns request but for our purposes we are only gonig to use these 2
#include <stddef.h>
#include <stdint.h>

#define MAX_DNS_NAME 256  // dotted hostname plus terminating NUL
#define MAX_IP_STR   16   // Max IPv4 string ("255.255.255.255\0") could also use built in INET_ADDRSTRLEN

typedef struct {
    char name[MAX_DNS_NAME];
} DnsQuestion;

typedef struct {
    char name[MAX_DNS_NAME];
    char data[MAX_IP_STR]; // IPv4 address
} DnsAnswer;

typedef struct {
    uint16_t id;
    uint16_t question_count;
    uint16_t answer_count;
    DnsQuestion question;
    DnsAnswer answer;
} DnsRequest;

typedef struct dns_record {
    char hostname[MAX_DNS_NAME];
    char ip[MAX_IP_STR];
} dns_record;

typedef struct dns_table {
    dns_record *records;
    size_t count;
    size_t capacity;
} dns_table;

// Returns 0 on success, -1 if invalid, already registered, or full.
int register_ip(dns_table *table, const char *hostname, const char *ip);

// Returns the IP string, or NULL if the hostname is not registered.
const char *lookup(const dns_table *table, const char *hostname);

// Returns 1 if this IP is unused, 0 if already registered.
int check_ownership(const dns_table *table, const char *ip);

// Allocates the record array; returns 0 on success, -1 on failure.
int dns_table_init(dns_table *table, size_t capacity);

// Looks up a hostname through the custom DNS server at dns_server_ip.
int dns_resolve(const char *dns_server_ip, const char *hostname,
                char *ip_buffer, size_t ip_buffer_size);

// deserialize dns request
// Custom packet: ID, question count, answer count, then labeled names and IPv4 data.
int deserialize_dns(const unsigned char *buffer, size_t buffer_len, DnsRequest *req);

// Returns serialized byte count, or -1 if invalid or buffer too small.
int serialize_dns(unsigned char *buffer, size_t buffer_size, const DnsRequest *req);
