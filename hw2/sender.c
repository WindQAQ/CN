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
	int v;
	if ((v = find_arg(argc, argv, "-dest_ip")) != -1)	strcpy(destIP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-dest_port")) != -1)	destPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-src_port")) != -1)	srcPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-file")) != -1)	strcpy(file_path, argv[v+1]);

	int socket_fd;
	struct sockaddr_in my_addr, agent_addr;
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < -1) {
		die("socket failed");
	}

	/* bind socket */
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(srcPort);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		die("bind failed");
	}

	/* info of agent */
	memset((char *)&agent_addr, 0, sizeof(agent_addr));
	agent_addr.sin_family = AF_INET;
	agent_addr.sin_port = htons(AGENT_SENDER_PORT);
	agent_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int agent_addr_len = sizeof(agent_addr);
	
	int fd = open(file_path, O_RDONLY);
	char data[BUF_LEN];
	int nbytes;

	char rcv_pkt[PKT_LEN], sd_pkt[PKT_LEN];
	Header sd_h;
	int rcv_len;
	while (1) {
		memset(data, 0, BUF_LEN);
		if ((nbytes = read(fd, data, BUF_LEN)) < 0) die("read file failed");
		if (nbytes == 0) break;

		make_pkt(&sd_h, data, nbytes, sd_pkt);
	
		if (sendto(socket_fd, sd_pkt, PKT_LEN, 0, (struct sockaddr*)&agent_addr, agent_addr_len) < 0) {
			die("sendto failed");
		}
	
		if ((rcv_len = recvfrom(socket_fd, rcv_pkt, PKT_LEN, 0, (struct sockaddr*)&agent_addr, &agent_addr_len)) < 0) {
			die("recvfrom failed");
		}
		printf("%s\n", rcv_pkt);
	}

	close(socket_fd);

	return 0;
}
