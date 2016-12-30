#include "headers.h"

int main(int argc, char **argp) {

	struct sockaddr_in dns_server;

	char buf[BUF_SIZE],host_name[64], dns_ip[16], host_ip[16];

	unsigned int i,s,sec,retry, ipaddr_len, req_type, dns_req_len=0, dns_resp_len=0;
	int ret;

	sec = 0;
	if(argc < 3) {
		printf("Invalid arguments,\nUsage:\n %s -w <host address>\nOR\n"
				" %s -d <ipv4_address>\n",
				argp[0], argp[0]);
		return -1;
	} else {
		for (i=1; i<argc; i+=2) {
			if (!strcmp(argp[i], "-w")) {
				strcpy(host_name, argp[i+1]);
				req_type = 0;
			} else if (!strcmp(argp[i], "-d")) {
				strcpy(host_ip, argp[i+1]);
				req_type = 1;
			} else if (!strcmp(argp[i], "-t")) {
				sec = atoi(argp[i+1]);
			} else {
				printf("Invalid arguments,\nUsage:\n %s -w <host address>\nOR\n"
						" %s -d <ipv4_address>\n",
						argp[0], argp[0]);
				return -1;
			}
		}
		if (sec < 2)
			sec = 2;
	}

	ret = getDnsServer(dns_ip);
	ret = fillDnsDetails(&dns_server, dns_ip);
	ipaddr_len = sizeof(dns_server);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(s == -1) {
		printf("Socket creation failed, unable to continue\n");
		return -1;
	}

	dns_req_len = fillDnsQuery(buf, req_type, host_name, host_ip);
	retry = 0;
	while(retry < MAX_ATTEMPTS) {
		ret = sendto(s, buf, dns_req_len, 0,
				(struct sockaddr *) &dns_server, ipaddr_len);
		if (ret == -1) {
			printf("SENDTO failed(DNS REQUEST), please check for possible link down in system and retry\n");
			return -1;
		}

		ret = waitForDnsReply(s, sec);
		if (ret == 0) {
			printf("Reply not yet received for %d secs\n", sec);
			retry++;
		} else {
			break;
		}
		printf("Retransmitting dns packet\n");
	}

	if (retry == MAX_ATTEMPTS) {
		printf("DNS server is unable to resolve or dns server is unreachable\n");
		printf("\nTo debug, try to increase waiting time with -t option\n");
		printf("%s %s %s -t <no of secs to wait>\n",
				argp[0],
				argp[1],
				((req_type) ? host_ip : host_name));
		return -1;
	}

	ret = recvfrom(s, buf, BUF_SIZE, 0,
			(struct sockaddr *) &dns_server, &ipaddr_len);
	if (ret == -1) {
		printf("RECVFROM failed(reading from socket\n");
		return -1;
	}

	dns_resp_len = ret;
	retrieveDnsResonse(buf, dns_req_len, dns_resp_len, req_type, host_ip, host_name);

	return 0;
}
