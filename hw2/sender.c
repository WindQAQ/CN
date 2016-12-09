#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "packet.h"

#define SENDER_PORT 12010
#define AGENT_PORT 12006
const char* LOCAL_HOST = "127.0.0.1";

char destIP[IP_LEN];
char destPort[BUF_LEN];
char file_path[BUF_LEN];
static int seq = 1;

void die(const char* err)
{
	perror(err);
	exit(1);
}

int find_arg(const int argc, char *argv[], const char* str)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(str, argv[i])) {
			if (i == argc-1) {
				fprintf(stderr, "No argument given for %s\n", str);
				exit(1);
			}
			return i;
		}
	}

	return -1;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "Usage options:\n");
		fprintf(stderr, "\t-ip <IP address>\n");
		fprintf(stderr, "\t-port <port number>\n");
		fprintf(stderr, "\t-file <path of file>\n");
		exit(1);
	}
	else {
		int v;
		if ((v = find_arg(argc, argv, "-ip")) != -1) strcpy(destIP, argv[v+1]);
		if ((v = find_arg(argc, argv, "-port")) != -1) strcpy(destPort, argv[v+1]);
		if ((v = find_arg(argc, argv, "-file")) != -1) strcpy(file_path, argv[v+1]);
	}

#ifdef DEBUG
	printf("Configuration: \n");
	printf("\tIP address: %s, port number: %s, file path: %s\n\n", IP_addr, port_num, file_path);
#endif

	int senderSocket;
	char senderIP[IP_LEN];
	struct sockaddr_in senderAddr, agentAddr;
	if ((senderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) die("Create socket error");
	
	/* bind socket */
	memset((char *)&senderAddr, 0, sizeof(senderAddr));
	senderAddr.sin_family = AF_INET;
	senderAddr.sin_port = htons(SENDER_PORT);
	senderAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, IP_LEN);
	if (bind(senderSocket, (struct sockaddr*) &senderAddr, sizeof(senderAddr)) < 0) die("Bind socket error");

	/* info of agent */
	memset((char *)&agentAddr, 0, sizeof(agentAddr));
	agentAddr.sin_family = AF_INET;
	agentAddr.sin_port = htons(AGENT_PORT);
	agentAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int src_fd = open(file_path, O_RDONLY);
	char src_bytes[BUF_LEN], recv_pkt[PKT_LEN];
	int src_len, recv_len;
	int addrlen = sizeof(agentAddr);

	char send_pkt[PKT_LEN];
	Header sdr_h;
	strcpy(sdr_h.srcIP, senderIP);
	sdr_h.srcPort = SENDER_PORT;
	strcpy(sdr_h.destIP, destIP);
	sdr_h.destPort = atoi(destPort);
	sdr_h.type = DATA;
	while (1) {
		src_len = read(src_fd, src_bytes, BUF_LEN);
		if (src_len < 0) die("Read source file error");
		if (src_len == 0) break;
		//printf("%s", src_bytes);
		sdr_h.seq = seq;
		make_packet(&sdr_h, src_bytes, send_pkt);
		seq++;
		if (sendto(senderSocket, send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&agentAddr, addrlen) < 0) {
			die("Send to agent error");
		}
		if ((recv_len = recvfrom(senderSocket, recv_pkt, PKT_LEN, 0, (struct sockaddr*)&agentAddr, &addrlen)) < 0) {
			die("Receive from agent error");
		}
		if (recv_len == 0) continue;
		printf("Receive from agent: %d\n", ntohs(agentAddr.sin_port));
		print_packet(recv_pkt);
	}

	close(senderSocket);
	return 0;
}
