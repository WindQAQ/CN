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

char IP_addr[BUF_LEN];
char port_num[BUF_LEN];
char file_path[BUF_LEN];

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
		if ((v = find_arg(argc, argv, "-ip")) != -1) strcpy(IP_addr, argv[v+1]);
		if ((v = find_arg(argc, argv, "-port")) != -1) strcpy(port_num, argv[v+1]);
		if ((v = find_arg(argc, argv, "-file")) != -1) strcpy(file_path, argv[v+1]);
	}

	printf("Configuration: \n");
	printf("\tIP address: %s, port number: %s, file path: %s\n\n", IP_addr, port_num, file_path);

	int agentSocket;
	struct sockaddr_in senderAddr, agentAddr;
	if ((agentSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) die("Create socket error");
	
	/* bind socket */
	memset((char *)&agentAddr, 0, sizeof(agentAddr));
	agentAddr.sin_family = AF_INET;
	agentAddr.sin_port = htons(AGENT_PORT);
	inet_pton(AF_INET, LOCAL_HOST, &(agentAddr.sin_addr));
	if (bind(agentSocket, (struct sockaddr*) &agentAddr, sizeof(agentAddr)) < 0) die("Bind socket error");

	/* info of sender */
	memset((char *)&agentAddr, 0, sizeof(senderAddr));
	senderAddr.sin_family = AF_INET;
	senderAddr.sin_port = htons(SENDER_PORT);
	inet_pton(AF_INET, LOCAL_HOST, &(senderAddr.sin_addr));

	char recv_pkt[PKT_LEN];
	int recv_len;
	int addrlen = sizeof(senderAddr);
	while (1) {
		if ((recv_len = recvfrom(agentSocket, recv_pkt, PKT_LEN, 0, (struct sockaddr*)&senderAddr, &addrlen)) < 0) {
			die("Receive from agent error");
		}
		printf("%s", recv_pkt);
		if (sendto(agentSocket, recv_pkt, recv_len, 0, (struct sockaddr*)&senderAddr, addrlen) < 0) {
			die("Send to agent error");
		}
	}

	close(agentSocket);
	return 0;
}
