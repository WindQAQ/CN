#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "agent.h"
#include "packet.h"

char destIP[BUF_LEN];
int destPort, srcPort;
char file_path[BUF_LEN];

int main(int argc, char* argv[])
{
	/* 0 for sender, 1 for receiver */
	int socket_fd[2];
	struct sockaddr_in agent_addr[2];

	/* create and bind socket */
	const int PORT[] = {AGENT_SENDER_PORT, AGENT_RECEIVER_PORT};
	for (int i = 0; i < sizeof(PORT)/sizeof(PORT[0]); i++) {
		if ((socket_fd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < -1) {
			die("socket[0] failed");
		}
		memset((char *)&agent_addr[i], 0, sizeof(agent_addr[i]));
		agent_addr[i].sin_family = AF_INET;
		agent_addr[i].sin_port = htons(PORT[i]);
		agent_addr[i].sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(socket_fd[i], (struct sockaddr *)&agent_addr[i], sizeof(agent_addr[i])) < 0) {
			die("bind failed");
		}
	}

	char rcv_pkt[PKT_LEN];
	int rcv_len;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	while (1) {
		if ((rcv_len = recvfrom(socket_fd[0], rcv_pkt, PKT_LEN, 0, (struct sockaddr*)&addr, &addr_len)) < 0) {
			die("recvfrom failed");
		}
		printf("%s\n", rcv_pkt);
		if (sendto(socket_fd[0], rcv_pkt, rcv_len, 0, (struct sockaddr*)&addr, addr_len) < 0) {
			die("sendto failed");
		}

	}

	close(socket_fd[0]);
	close(socket_fd[1]);

	return 0;
}
