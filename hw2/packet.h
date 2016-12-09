#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#define BUF_LEN 1024
#define IP_LEN 32
#define PKT_LEN 4096
#define HEADER_LEN 24

#define DATA 0
#define ACK 1
#define FIN 2
#define FINACK 3

typedef struct header {
	int srcPort;
} Header;

void make_pkt(Header* h, char *data, int len, char *pkt)
{
	memset(pkt, 0, PKT_LEN);
	memcpy(pkt, data, len);
}
