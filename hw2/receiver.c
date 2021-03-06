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

typedef struct buffer {
	char data[BUF_LEN];
	int len;
	int used;
} Buffer;

char IP[BUF_LEN];
int port, buffer_size = 32;
char file_path[BUF_LEN];

Buffer *buffer;
int total_used = 0;
int base = 1, seq = 1;

int main(int argc, char* argv[])
{
	int v;
    if ((v = find_arg(argc, argv, "ip")) != -1) strcpy(IP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-port")) != -1) port = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-file")) != -1) strcpy(file_path, argv[v+1]);
	if ((v = find_arg(argc, argv, "-buffer")) != -1) buffer_size = atoi(argv[v+1]);

	buffer = calloc(buffer_size, sizeof(Buffer));

	int socket_fd;
	struct sockaddr_in my_addr;
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		die("socket failed");
	}

	/* bind socket */
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
    inet_pton(AF_INET, IP, &my_addr.sin_addr);
	if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		die("bind failed");
	}

	/* open destination file */
	int fd;
	if ((fd = open(file_path, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) die("open file failed");
	int rcv_len;

	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	Packet rcv_pkt, snd_pkt;
	int type;
	while (1) {
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&addr, &addr_len)) < 0) {
			die("recvfrom failed");
		}
		
		type = rcv_pkt.h.type;
		seq = rcv_pkt.h.seq;

		if (type == DATA) {
			if ((seq-base >= buffer_size) || (seq-base < 0)) {
				/* out of range */
				printf("drop\tdata\t#%d\n", seq);
				if (total_used == buffer_size) {
					printf("flush\n");
					for (int i = 0; i < buffer_size; i++)
						if (write(fd, buffer[i].data, buffer[i].len) < 0) die("write to file failed");
					base += buffer_size;
					memset(buffer, 0, buffer_size*sizeof(Buffer));
					total_used = 0;
				}
				continue;
			}
			else if (buffer[seq-base].used == 1) {
				/* already received */
				printf("ignr\tdata\t#%d\n", seq);
				snd_pkt.h.type = ACK;
			}
			else {
				/* ack it */
				printf("recv\tdata\t#%d\n", seq);
				memcpy(buffer[seq-base].data, rcv_pkt.data, rcv_pkt.h.len);
				buffer[seq-base].len = rcv_pkt.h.len;
				buffer[seq-base].used = 1;
				snd_pkt.h.type = ACK;
				total_used += 1;
			}
		}
		else if (type == FIN) {
			printf("recv\tfin\n");
			snd_pkt.h.type = FINACK;
		}

		memcpy(&snd_pkt.h.src, &rcv_pkt.h.dest, sizeof(struct sockaddr_in));
		memcpy(&snd_pkt.h.dest, &rcv_pkt.h.src, sizeof(struct sockaddr_in));
		snd_pkt.h.len = 0;
		snd_pkt.h.seq = seq;
		memset(snd_pkt.data, 0, BUF_LEN*sizeof(char));
		if (sendto(socket_fd, &snd_pkt, sizeof(Packet), 0, (struct sockaddr*)&addr, addr_len) < 0) {
			die("sendto failed");
		}
		if (snd_pkt.h.type == ACK) {
			printf("send\tack\t#%d\n", snd_pkt.h.seq);
		}
		else if (snd_pkt.h.type == FINACK) {
			printf("send\tfinack\n");
			break;
		}
	}

	for (int i = 0; buffer[i].used == 1 && i < buffer_size; i++) {
		if (i == 0) printf("flush\n");
		if (write(fd, buffer[i].data, buffer[i].len) < 0) die("write to file failed");
	}
	
	free(buffer);
	close(socket_fd);

	return 0;
}
