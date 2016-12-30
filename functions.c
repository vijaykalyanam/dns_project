#include "headers.h"

void retrieveDnsResponseName(char *resp, char *buf, unsigned int len) {
	char *p = resp, *q;
	unsigned int i,j,count;

	for(i=0; i<len; i++) {
		count = resp[i];
		for(j=0; j<count; j++,i++) buf[i] = resp[i+1];
		if(i<len) buf[i+1]='.';
	}
	buf[i]='\0';
	return;
}

int retrieveDnsResonse(char *buf,
		unsigned short req_len,
		unsigned short response_len,
		int req_type,
		char *ip,
		char *domain_name) {

	unsigned short int i = req_len, len, type, flags, rcode, mask = 0x000f;

	struct dns_response *resp;
	struct in_addr ip_inet = {0};
	char resolve_buf[128];
	struct dns_request *req;

	req = (struct dns_request *) buf;
	flags = ntohs(req->header.flags);
	rcode = flags&mask;
	if (rcode == NAME_ERROR) {
		printf("Authoritative Name Server returned that the\n"
				"Domain name/IP specified in the query doesn't exist\n");
		return 0;
	}

	memset(resolve_buf, '.', 128);
	buf += i; 
	do {
		resp = (struct dns_response *) buf;
		len = ntohs(resp->data_len);
		if(!req_type && (ntohs(resp->type) == TYPE_IP)) {
			ip = buf + sizeof(struct dns_response);
			ip_inet = *((struct in_addr *)ip);
			printf("IP Address for %s is %s\n", domain_name, inet_ntoa(ip_inet));
		} else if (req_type && (ntohs(resp->type) == TYPE_PTR)) {
			retrieveDnsResponseName(buf+sizeof(struct dns_response), resolve_buf, len);
			printf("Reverse mapping Address for %s is %s\n", ip, resolve_buf);
			return 0;	
		}
		buf += (len + sizeof(struct dns_response));
		i += (len+sizeof(struct dns_response));
	}while(i < response_len);

	return 0;
}

char *fillDnsQueryAddr(char *p, char *q) {
	char *r;
	unsigned short count=0;

	r=p+1;//to fill label count

	while(*q) {
		if(*q == '.') {
			*p=count;
			count = 0;
			p=r;
		} else {
			*r = *q;
			count++;
		}
		r++;
		q++;
	}
	*p=count;
	*r=0;

	return (r+1);
}

void reverse(char *p, int j) {
	char ch;
	unsigned short int i=0;
	while(i<=j) {
		ch = p[i];
		p[i]=p[j];
		p[j]=ch;
		i++;
		j--;
	}
}

void find_word(char *str, const int *i, int *j) {
        unsigned short int  q=*i;

        while(str[q]) {
                if(str[q] == '.') {
                        break;
                }
                q++;
        }
        *j=q-1;
}

void changeIpFormat(char *str) {
	int i=0,j=0,pos,len=0;
	while(str[len++]!='\0');len--;
	reverse(str, len-1);

	while(str[i]!='\0') {
		find_word(str, &i, &j);
		reverse(str+i, j-i);
		if(str[j+1]=='.')
			i=j+2;
		else
			i = j+1;
	}

	return;
}


int fillDnsQuery(char *buf, unsigned short int req_type, char *hostaddr, char *hostip) {

	struct dns_request *req;
	struct query_type *qt;
	char ip_buf[32]={0}, *r;
	unsigned short dns_req_len=0;

	bzero(buf, BUF_SIZE);

	req = (struct dns_request *) buf;
	req->header.identifier = htons(12345);
	req->header.flags = htons(0x0100);
	req->options.no_of_questions = htons(1);

	if (req_type) {
		strcpy(ip_buf, hostip);
		printf("ip_buf :%s\n", ip_buf);
		changeIpFormat(ip_buf);
		printf("ip_buf :%s\n", ip_buf);
		strcpy(ip_buf+strlen(hostip), ".in-addr.arpa");
		printf("ip_buf :%s\n", ip_buf);
		r = fillDnsQueryAddr(&req->data, ip_buf);
	} else {
		r = fillDnsQueryAddr(&req->data, hostaddr);
	}

	qt = (struct query_type *)(r);
	if(req_type)
		qt->qtype = 0x0c;
	else	
		qt->qtype = 0x01;
	qt->qclass = 0x01;

	dns_req_len = (char *) &qt->qclass - buf; 
	if(dns_req_len < 0)
		dns_req_len = dns_req_len*(-1);
	dns_req_len += 1;

	return dns_req_len;
}

int fillDnsDetails(struct sockaddr_in *dns_server, char *ip_addr) {

	memset(dns_server, 0, sizeof(struct sockaddr_in));

	dns_server->sin_family=AF_INET;
	dns_server->sin_addr.s_addr=(inet_addr(ip_addr));
	dns_server->sin_port=htons(DNS_SERVER);

	return 0;
}

int getDnsServerIp(char *str, char *addr) {

	while(*str) {
		if(*str == '#') {
			while(*str != '\n') str++;
		} else if (*str == 'n') {
			while(*str != ' ') str++; str++;
			while(*str != '\n') *addr++ = *str++;
			if(*str == '\n') {
				*addr = '\0';
				break;
			}			else {
				printf("error in reading /etc/resolve.conf file\n");
				return -1;
			} 
		}
		str++;
	}

	return 0;
}

int getDnsServer(char *addr) {
	int ret,i=0;
	char ch,str[100];
	FILE *fp;

	fp = fopen("/etc/resolv.conf", "r");
	if(fp == NULL) {
		printf("Error in file opening\n");
		return;
	}
	while((ch=fgetc(fp)) != EOF) { 
		str[i++] = ch;
	}
	fclose(fp);
	str[i]='\0';
	printf("FILE :\n%s\n", str);
	getDnsServerIp(str, addr);
	printf("DNS IP ADDR : %s\n", addr);
	return 0;
}
