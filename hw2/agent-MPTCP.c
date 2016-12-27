#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "packet.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int total_data = 0, total_drop = 0;
double loss_prob = 0.0, loss_rate = 0.0;

void* agent(void* _fd)
{
	int socket_fd = *(int*) _fd;

	srand(time(NULL) * socket_fd);

	Packet rcv_pkt;
	int rcv_len;
	struct sockaddr_in src_addr, dest_addr;
	int src_addr_len = sizeof(src_addr), dest_addr_len = sizeof(dest_addr);
	int type;
	int dropped = 0;
	while (1) {
		/* receive packets */
		if ((rcv_len = recvfrom(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&src_addr, &src_addr_len)) < 0) {
			die("recvfrom failed");
		}
		pthread_mutex_lock(&m);
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
			printf("fwd\tdata\t#%d,\tloss rate = %f%s\n", rcv_pkt.h.seq, loss_rate, (dropped)? ", drop\t": "");
			if (dropped) {
				pthread_mutex_unlock(&m);
				continue;
			}
		}
		else if (type == ACK) {
			printf("fwd\tack\t#%d\n", rcv_pkt.h.seq);
		}
		else {
			printf("fwd\t%s\n", TYPE[type]);
		}
		pthread_mutex_unlock(&m);
		memcpy(&dest_addr, &rcv_pkt.h.dest, sizeof(struct sockaddr_in));
		if (sendto(socket_fd, &rcv_pkt, sizeof(Packet), 0, (struct sockaddr*)&dest_addr, dest_addr_len) < 0) {
			die("sendto failed");
		}
	}

	pthread_exit(NULL);	
}

int main(int argc, char* argv[])
{
	int v;
	int subflow_num = 1;
	int *port; 
	char subflowIP[IP_LEN];
	if ((v = find_arg(argc, argv, "-loss_prob")) != -1) loss_prob = atof(argv[v+1]);
	if ((v = find_arg(argc, argv, "-subflow_num")) != -1) subflow_num = atoi(argv[v+1]);	
	if ((v = find_arg(argc, argv, "-ip")) != -1) strcpy(subflowIP, argv[v+1]);
	port = (int *) malloc(subflow_num * sizeof(int));
	if ((v = find_arg(argc, argv, "-port")) != -1) {
		for (int i = 0; i < subflow_num; i++) {
			port[i] = atoi(argv[v+i+1]);
		}
	}
	else {
		for (int i = 0; i < subflow_num; i++) {
			port[i] = 12040+i;
		}
	}

	int* socket_fd;
	struct sockaddr_in* subflow_addr;

	socket_fd = (int *) malloc(subflow_num * sizeof(int));
	subflow_addr = (struct sockaddr_in *) calloc(subflow_num, sizeof(struct sockaddr_in));

	/* create and bind socket */
	pthread_t *tid = (pthread_t*) malloc(subflow_num * sizeof(pthread_t));
	for (int i = 0; i < subflow_num; i++) {
		if ((socket_fd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			die("socket failed");
		}
		subflow_addr[i].sin_family = AF_INET;
		subflow_addr[i].sin_port = htons(port[i]);
		inet_pton(AF_INET, subflowIP, &(subflow_addr[i].sin_addr));
		if (bind(socket_fd[i], (struct sockaddr *)&subflow_addr[i], sizeof(subflow_addr[i])) < 0) {
			die("bind failed");
		}
		pthread_create(&tid[i], NULL, &agent, &socket_fd[i]);
	}

	for (int i = 0; i < subflow_num; i++) {
		pthread_join(tid[i], NULL);
		close(socket_fd[i]);
	}

	free(port);
	free(socket_fd);
	free(subflow_addr);
	free(tid);

	return 0;
}
