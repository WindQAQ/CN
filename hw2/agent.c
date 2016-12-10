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
	int socket_fd;
	struct sockaddr_in my_addr;

	/* create and bind socket */
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		die("socket failed");
	}
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(AGENT_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		die("bind failed");
	}

	Packet rcv_pkt;
	int rcv_len;
	struct sockaddr_in src_addr, dest_addr;
	int src_addr_len = sizeof(src_addr);
	int dest_addr_len = sizeof(dest_addr);
	int type;
	while (1) {
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&src_addr, &src_addr_len)) < 0) {
			die("recvfrom failed");
		}
		type = rcv_pkt.h.type;
		if ((type == DATA) || type == ACK) {
			printf("get\t%s\t#%d\n", TYPE[type], rcv_pkt.h.seq);
		}
		else {
			printf("get\t%s\n", TYPE[type]);
		}

		memcpy(&dest_addr, &rcv_pkt.h.dest, sizeof(struct sockaddr_in));
		if (sendto(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&dest_addr, dest_addr_len) < 0) {
			die("sendto failed");
		}
		if ((type == DATA) || type == ACK) {
			printf("fwd\t%s\t#%d\n", TYPE[type], rcv_pkt.h.seq);
		}
		else {
			printf("fwd\t%s\n", TYPE[type]);
		}
	}

	close(socket_fd);

	return 0;
}
