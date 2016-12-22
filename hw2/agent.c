#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "packet.h"

char IP[BUF_LEN];
int port;
char file_path[BUF_LEN];

int main(int argc, char* argv[])
{
	int v;
	double loss_prob = 0.0;
	strcpy(IP, "127.0.0.1");
	if ((v = find_arg(argc, argv, "-loss_prob")) != -1) loss_prob = atof(argv[v+1]);
	if ((v = find_arg(argc, argv, "-ip")) != -1) strcpy(IP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-port")) != -1) port = atoi(argv[v+1]);

	int socket_fd;
	struct sockaddr_in my_addr;

	/* create and bind socket */
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		die("socket failed");
	}
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	inet_pton(AF_INET, IP, &(my_addr.sin_addr));
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		die("bind failed");
	}

	srand(time(NULL));
	int total_data = 0, total_drop = 0, dropped = 0;
	double loss_rate = 0.0;

	Packet rcv_pkt;
	int rcv_len;
	struct sockaddr_in src_addr, dest_addr;
	int src_addr_len = sizeof(src_addr);
	int dest_addr_len = sizeof(dest_addr);
	int type;
	while (1) {
		/* receiver packets */
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

		/* forward packets */
		if (type == DATA) {
			dropped = 0;
			total_data++;
			if ((double)rand()/RAND_MAX < loss_prob) {
				dropped = 1;
				total_drop++;
			} 
			loss_rate = (double) total_drop/total_data;
			printf("fwd\tdata\t#%d,\tloss rate = %f%s\n", rcv_pkt.h.seq, loss_rate, (dropped)? ", drop": "");
			if (dropped) {
				continue;
			}
		}
		else if (type == ACK) {
			printf("fwd\tack\t#%d\n", rcv_pkt.h.seq);
		}
		else {
			printf("fwd\t%s\n", TYPE[type]);
		}
		memcpy(&dest_addr, &rcv_pkt.h.dest, sizeof(struct sockaddr_in));
		if (sendto(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&dest_addr, dest_addr_len) < 0) {
			die("sendto failed");
		}
	}

	close(socket_fd);

	return 0;
}
