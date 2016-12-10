#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "packet.h"

int port;
char file_path[BUF_LEN];

int main(int argc, char* argv[])
{
	int v;
	if ((v = find_arg(argc, argv, "-port")) != -1) port = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-file")) != -1) strcpy(file_path, argv[v+1]);

	int socket_fd;
	struct sockaddr_in my_addr;
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		die("socket failed");
	}

	/* bind socket */
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		die("bind failed");
	}

	int fd;
	if ((fd = open(file_path, O_WRONLY | O_CREAT, 0644)) < 0) die("open file failed");
	int rcv_len;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	Packet rcv_pkt, sd_pkt;
	int type;
	while (1) {
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&addr, &addr_len)) < 0) {
			die("recvfrom failed");
		}
		
		type = rcv_pkt.h.type;
		if (type == DATA) {
			printf("recv\tdata\t#%d\n", rcv_pkt.h.seq);
			if (write(fd, rcv_pkt.data, rcv_pkt.h.len) < 0) die("write to file failed");
			sd_pkt.h.type = ACK;		
		}
		else if (type == FIN) {
			sd_pkt.h.type = FINACK;
		}
		else {
			fprintf(stderr, "receive strange type: %s\n", TYPE[sd_pkt.h.type]);
			exit(1);
		}

		memcpy(&sd_pkt.h.src, &rcv_pkt.h.dest, sizeof(struct sockaddr_in));
		memcpy(&sd_pkt.h.dest, &rcv_pkt.h.src, sizeof(struct sockaddr_in));
		sd_pkt.h.len = 0;
		sd_pkt.h.seq = rcv_pkt.h.seq;
		memset(sd_pkt.data, 0, BUF_LEN);
		if (sendto(socket_fd, &sd_pkt, sizeof(Packet), 0, (struct sockaddr*)&addr, addr_len) < 0) {
			die("sendto failed");
		}
		if (sd_pkt.h.type == ACK) {
			printf("send\tack\t#%d\n", sd_pkt.h.seq);
		}
		else if (sd_pkt.h.type == FINACK) {
			printf("send\tfinack\n");
			break;
		}
		else {
			fprintf(stderr, "send strange type: %s\n", TYPE[sd_pkt.h.type]);
			exit(1);
		}
	}

	close(socket_fd);

	return 0;
}
