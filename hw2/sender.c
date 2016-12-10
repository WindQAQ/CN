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
	if ((v = find_arg(argc, argv, "-destIP")) != -1)	strcpy(destIP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-destPort")) != -1)	destPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-srcPort")) != -1)	srcPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-file")) != -1)	strcpy(file_path, argv[v+1]);

	int socket_fd;
	struct sockaddr_in my_addr, dest_addr, agent_addr;
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
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
	agent_addr.sin_port = htons(AGENT_PORT);
	agent_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int agent_addr_len = sizeof(agent_addr);
	
	/* info of destinaion */
	memset((char *)&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(destPort);
	inet_pton(AF_INET, destIP, &(dest_addr.sin_addr));

	/* Construct header of packet*/
	Packet rcv_pkt, sd_pkt;
	Header sd_h;
	memset(&sd_h, 0, sizeof(Header));
	memcpy(&sd_h.src, &my_addr, sizeof(struct sockaddr_in));
	memcpy(&sd_h.dest, &dest_addr, sizeof(struct sockaddr_in));
	sd_h.type = DATA;
	sd_h.seq = 0;

	int fd;
	if ((fd = open(file_path, O_RDONLY)) < 0) die("open file failed");
	char data[BUF_LEN];
	int nbytes;

	int rcv_len;
	while (1) {
		memset(data, 0, BUF_LEN);
		if ((nbytes = read(fd, data, BUF_LEN)) < 0) die("read file failed");
		if (nbytes == 0) break;

		make_pkt(&sd_h, data, nbytes, &sd_pkt);	
		print_pkt(&sd_pkt);

		if (sendto(socket_fd, &sd_pkt, sizeof(Packet), 0, (struct sockaddr*)&agent_addr, agent_addr_len) < 0) {
			die("sendto failed");
		}
	
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&agent_addr, &agent_addr_len)) < 0) {
			die("recvfrom failed");
		}
		print_pkt(&rcv_pkt);
	}

	close(socket_fd);

	return 0;
}
