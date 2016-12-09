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

#define RECEIVER_PORT 12012
#define AGENT_PORT 12004
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
#ifdef ARG
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
#endif

#ifdef DEBUG
	printf("Configuration: \n");
	printf("\tIP address: %s, port number: %s, file path: %s\n\n", IP_addr, port_num, file_path);
#endif

	int receiverSocket;
	char receiverIP[IP_LEN];
	struct sockaddr_in receiverAddr;
	if ((receiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) die("Create socket error");
	
	/* bind socket */
	memset((char *)&receiverAddr, 0, sizeof(receiverAddr));
	receiverAddr.sin_family = AF_INET;
	receiverAddr.sin_port = htons(RECEIVER_PORT);
	receiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_ntop(AF_INET, &(receiverAddr.sin_addr), receiverIP, IP_LEN);
	if (bind(receiverSocket, (struct sockaddr*) &receiverAddr, sizeof(receiverAddr)) < 0) die("Bind socket error");

	struct sockaddr_in agentAddr;
	int agentAddr_len = sizeof(agentAddr);

	char send_pkt[PKT_LEN], recv_pkt[PKT_LEN];
	char send_data[BUF_LEN], recv_data[BUF_LEN];
	int recv_len;
	Header sdr_h, recv_h;
	strcpy(recv_h.srcIP, receiverIP);
	recv_h.srcPort = RECEIVER_PORT;
	recv_h.type = ACK;
	recv_h.seq = 0;
	while (1) {
		if ((recv_len = recvfrom(receiverSocket, recv_pkt, PKT_LEN, 0, (struct sockaddr*)&agentAddr, &agentAddr_len)) < 0) {
			die("Receive from agent error");
		}
		if (recv_len == 0) continue;
		printf("Receive from agent: %d\n", ntohs(agentAddr.sin_port));
		printf("=====================================\n");
		extract_packet(&sdr_h, recv_data, recv_pkt);
		printf("%s\n", recv_data);	
		recv_h.destPort = sdr_h.srcPort;
		strcpy(recv_h.destIP, sdr_h.srcIP);
		make_packet(&recv_h, "Hello, I am receiver.\n", send_pkt);
		
		if (sendto(receiverSocket, send_pkt, sizeof(send_pkt), 0, (struct sockaddr*)&agentAddr, agentAddr_len) < 0) {
			die("Send to agent error");
		}
	}

	close(receiverSocket);
	return 0;
}
