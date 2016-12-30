#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <asm/byteorder.h>
#include <asm/types.h>

#define DNS_SERVER 53
#define BUF_SIZE 512
#define MAX_ATTEMPTS 3

enum {
	TYPE_IP = 0x01,
	TYPE_PTR = 0x0c,
	NAME_ERROR = 0x03
};

struct dns_header {
	unsigned short int identifier; //ID to track response
	unsigned short int flags;
};

struct query_type {
	char rsvd1;
	char qtype;
	char rsvd2;
	char qclass;
};

struct dns_options {
	unsigned short int no_of_questions;
	unsigned short int no_of_resource_records;
	unsigned short int no_of_authority_rrs;
	unsigned short int no_of_additional_rrs;
};

struct dns_request {
	struct dns_header header;
	struct dns_options options;
	char data;
};

struct dns_response {
	unsigned short int ptr;
	unsigned short int type;
	unsigned short int cl;
	char ttl[4];
	unsigned short int data_len;
};

