#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#define BUF_LEN 1024
#define IP_LEN 128
#define PKT_LEN 4096
#define HEADER_LEN 24

#define DATA 0
#define ACK 1
#define FIN 2
#define FINACK 3

const char* TYPE[] = {"data", "ack", "fin", "finack"};

typedef struct header {
	struct sockaddr_in src;
	struct sockaddr_in dest;
	int type;
	int seq;
	int len;
} Header;

typedef struct packet {
	Header h;
	char data[BUF_LEN];
} Packet;

void make_pkt(Header* h, char *data, int len, Packet* pkt)
{
	h->len = len;
	memset(pkt, 0, sizeof(Packet));
	memcpy(&pkt->h, h, sizeof(Header));
	memcpy(pkt->data, data, len);
}

void print_pkt(Packet* pkt)
{
	char srcIP[IP_LEN], destIP[IP_LEN];
	inet_ntop(AF_INET, &pkt->h.src.sin_addr, srcIP, IP_LEN);
	inet_ntop(AF_INET, &pkt->h.dest.sin_addr, destIP, IP_LEN);
	printf("\n===========================================\n");
	printf("srcIP: %s, srcPort: %d\n", srcIP, ntohs(pkt->h.src.sin_port));
	printf("destIP: %s, destPort: %d\n", destIP, ntohs(pkt->h.dest.sin_port));
	printf("type: %s, seq: %d, data length: %d\n", TYPE[pkt->h.type], pkt->h.seq, pkt->h.len);
	if (pkt->h.type == DATA) {
		printf("data: \n%s\n", pkt->data);
	}
}
