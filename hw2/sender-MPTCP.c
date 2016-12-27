#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>

#include "utility.h"
#include "packet.h"

typedef struct window {
	Packet pkt;
	int acked;
	int sent;
} Window;

char destIP[BUF_LEN], agentIP[BUF_LEN];
int destPort, srcPort, *agentPort, subflow_num, thres = 16;
char file_path[BUF_LEN];

int main(int argc, char* argv[])
{
	int v;
	if ((v = find_arg(argc, argv, "-destIP")) != -1)	strcpy(destIP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-destPort")) != -1)	destPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-srcPort")) != -1)	srcPort = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-file")) != -1)	strcpy(file_path, argv[v+1]);
	if ((v = find_arg(argc, argv, "-thres")) != -1) thres = atoi(argv[v+1]);
	if ((v = find_arg(argc, argv, "-subflow_num")) != -1) {
		subflow_num = atoi(argv[v+1]);
		agentPort = malloc(subflow_num * sizeof(int));
		for (int i = 0; i < subflow_num; i++) {
			agentPort[i] = 12040+i;
		}
	}
	if ((v = find_arg(argc, argv, "-agentIP")) != -1)   strcpy(agentIP, argv[v+1]);
	if ((v = find_arg(argc, argv, "-agentPort")) != -1) {
		for (int i = 0; i < subflow_num; i++) {
			agentPort[i] = atoi(argv[v+i+1]);
		}
	}

	int socket_fd;
	struct sockaddr_in my_addr, dest_addr, *agent_addr;
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
	agent_addr = calloc(subflow_num, sizeof(struct sockaddr_in));
	int *agent_addr_len = calloc(subflow_num ,sizeof(int));
	for (int i = 0; i < subflow_num; i++) {
		agent_addr[i].sin_family = AF_INET;
		agent_addr[i].sin_port = htons(agentPort[i]);
		inet_pton(AF_INET, agentIP, &(agent_addr[i].sin_addr));
		agent_addr_len[i] = sizeof(struct sockaddr_in);
	}
	
	/* info of destinaion */
	memset((char *)&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(destPort);
	inet_pton(AF_INET, destIP, &(dest_addr.sin_addr));

	/* construct header of packet */
	Header snd_h;
	memset(&snd_h, 0, sizeof(Header));
	memcpy(&snd_h.src, &my_addr, sizeof(struct sockaddr_in));
	memcpy(&snd_h.dest, &dest_addr, sizeof(struct sockaddr_in));
	snd_h.type = DATA;

	/* set receive timeout on socket: 1 second  */
	struct timeval timeout;
	memset(&timeout, 0, sizeof(struct timeval));
	timeout.tv_sec = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&timeout, sizeof(struct timeval))) die("setsockopt failed");

	/* open source file */
	int fd;
	if ((fd = open(file_path, O_RDONLY)) < 0) die("open file failed");
	char data[BUF_LEN];
	int nbytes;
	
	/* read data and pack data */
	int init_size = 2<<10;
	Window *snd_pkt = (Window*) malloc(init_size * sizeof(Window));
	int total_pkt = 0, size = init_size;
	while (1) {
		memset(data, 0, BUF_LEN*sizeof(char));
		if ((nbytes = read(fd, data, BUF_LEN)) < 0) die("read file failed");
		if (nbytes == 0) break;
		if (total_pkt == size) {
			size *= 2;
			Window *new_mem = realloc(snd_pkt, size*sizeof(Window));
			if (new_mem == NULL) {
				fprintf(stderr, "file is too large");
				exit(1);
			}
			snd_pkt = new_mem;
		}
		snd_h.seq = total_pkt + 1;
		make_pkt(&snd_h, data, nbytes, &snd_pkt[total_pkt].pkt);
		snd_pkt[total_pkt].acked = 0;
		snd_pkt[total_pkt].sent = 0;
		total_pkt++;
	}

	/* send data and receive ack */
	srand(time(NULL));
	int rcv_len;
	Packet rcv_pkt;
	int base = 1, win_size = 1, done = 0;
	struct sockaddr_in rcv_addr;
	int rcv_addr_len;
	while (!done) {
		/* send packets */
		int total_snd = 0;
		for (int i = base; i < base + win_size && i <= total_pkt; i++) {
			int rand_agent = (int)rand() % subflow_num;
			if (sendto(socket_fd, &snd_pkt[i-1].pkt, sizeof(Packet), 0, (struct sockaddr*)&agent_addr[rand_agent], agent_addr_len[rand_agent]) < 0) {
				die("sendto failed");
			}
			printf("%s\tdata\t#%d,\twinSize = %d,\tpath = %d\n", (snd_pkt[i-1].sent)? "resnd": "send", snd_pkt[i-1].pkt.h.seq, win_size, rand_agent);
			snd_pkt[i-1].sent = 1;
			total_snd++;
		}
		
		/* receive packets */
		int total_rcv = 0;
		while (total_rcv < total_snd) {
			if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&rcv_addr, &rcv_addr_len)) < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
				}
				else {
					die("recvfrom failed");
				}
			}
			if (rcv_len == 0) continue;
			int rcv_ack = rcv_pkt.h.seq;
			if (rcv_ack < base || rcv_ack >= base + win_size) {
				printf("ack out of range");
				continue;
			}
			printf("recv\tack\t#%d\n", rcv_ack);
			snd_pkt[rcv_ack-1].acked = 1;
			total_rcv++;
		}
		if (total_rcv != total_snd) {
			/* packet loss: timeout */
			thres = (win_size/2 > 1)? win_size/2: 1;
			win_size = 1;
			printf("time\tout,\t\tthreshold = %d\n", thres);
			/* find base */
			for (int i = base; i <= total_pkt; i++) {
				if (snd_pkt[i-1].acked == 0) {
					base = i;
					break;
				}
				if (i == total_pkt) done = 1;
			}
		}
		else {
			/* all packets within window are acked */
			base += win_size;
			win_size = (win_size < thres)? win_size*2: win_size+1;
			if (base > total_pkt) done = 1;
		}
	}

	/* send ack */
	Packet ack;
	snd_h.type = FIN;
	snd_h.seq = 0;
	make_pkt(&snd_h, NULL, 0, &ack);
	while (1) {
		int rand_agent = (int) rand() % subflow_num;
		if (sendto(socket_fd, &ack, sizeof(Packet), 0, (struct sockaddr*)&agent_addr[rand_agent], agent_addr_len[rand_agent]) < 0) {
			die("sendto failed");
		}
		printf("send\tfin\n");
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&rcv_addr, &rcv_addr_len)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			else {
				die("recvfrom failed");
			}
		}
		if (rcv_pkt.h.type == FINACK) {
			printf("recv\tfinack\n");
			break;
		}
		else {
			printf("stange type\n");
		}
	}

	free(snd_pkt);
	free(agent_addr);
	free(agent_addr_len);
	close(socket_fd);

	return 0;
}
