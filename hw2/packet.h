#include <string.h>
#include <stdlib.h>

#define DATA 0
#define ACK 1
#define FIN 2
#define FINACK 3

#define IP_LEN 256
#define BUF_LEN 1024
#define PKT_LEN 4096

typedef struct header {
	char srcIP[IP_LEN];
	int srcPort;
	char destIP[IP_LEN];
	int destPort;
	int type;
	int seq;
} Header;

void make_packet(Header* h, char* data, char* pkt)
{
	memset(pkt, 0, sizeof(pkt));
	sprintf(pkt, "%s\n%d\n%s\n%d\n%d\n%d\n%s", h->srcIP, h->srcPort, h->destIP, h->destPort, 
											   h->type, h->seq, data);
}

void print_packet(char* pkt)
{
	Header h;
	char *data;
	sscanf(pkt, "%s\n%d\n%s\n%d\n%d\n%d", 
                  h.srcIP, &h.srcPort, h.destIP, &h.destPort, &h.type, &h.seq);
	
	data = strchr(pkt, '\n');
	for (int i = 0; i < 5; i++)
		data = strchr(data+1, '\n');

	printf("srcIP: %s, srcPort: %d\n", h.srcIP, h.srcPort);
	printf("destIP: %s, destPort: %d\n", h.destIP, h.destPort);
	printf("type: %d, seq: %d\n", h.type, h.seq);
	printf("=============================================\n");
	printf("%s\n", data+1);
}

void extract_packet(Header* h, char* data, char* pkt)
{
	sscanf(pkt, "%s\n%d\n%s\n%d\n%d\n%d", 
                  h->srcIP, &h->srcPort, h->destIP, &h->destPort, &h->type, &h->seq);

	memset(data, 0, sizeof(data));
	data = strchr(pkt, '\n');
	for (int i = 0; i < 5; i++)
		data = strchr(data+1, '\n');
	data = data+1;
}
