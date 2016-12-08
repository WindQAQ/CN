#include <string.h>
#include <stdlib.h>

#define DATA 0
#define ACK 1
#define FIN 2
#define FINACK 3

#define IP_LEN 256
#define BUF_LEN 1024
#define PKT_LEN 4096

typedef struct packet {
	char srcIP[IP_LEN];
	int srcPort;
	char destIP[IP_LEN];
	int destPort;
	int type;
	int seq;
	char data[BUF_LEN];
} Packet;

void make_packet(int type, int seq, char *data, char* pkt)
{
	memset(pkt, 0, sizeof(pkt));
	sprintf(pkt, "%d\n%d\n%s", type, seq, data);
}

void print_packet(char* pkt)
{
	int type, seq;
	char *data;
	sscanf(pkt, "%d\n%d", &type, &seq);
	
	data = strchr(pkt, '\n');
	data = strchr(data+1, '\n');

	printf("Type: %d\n", type);
	printf("Seq: %d\n", seq);
	printf("Data:\n%s\n", data+1);
}

void extract_packet(char* pkt)
{

}
