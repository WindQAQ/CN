#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "packet.h"
#include "agent.h"

#define SENDER_PORT 12010
#define AGENT_PORT_FOR_SENDER 12006
#define AGENT_PORT_FOR_RECEIVER 12004

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
#ifdef ARG
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
#endif

#ifdef DEBUG
	printf("Configuration: \n");
	printf("\tIP address: %s, port number: %s, file path: %s\n\n", IP_addr, port_num, file_path);
#endif

	Agent agent;
	if ((agent.sender.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) die("Create socket for sender error");
	if ((agent.receiver.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) die("Create socket for receiver error");
	
	/* bind socket for sender */
	memset((char *)&agent.sender.addr, 0, sizeof(agent.sender.addr));
	agent.sender.addr.sin_family = AF_INET;
	agent.sender.addr.sin_port = htons(AGENT_PORT_FOR_SENDER);
	agent.sender.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(agent.sender.fd, (struct sockaddr*) &agent.sender.addr, sizeof(agent.sender.addr)) < 0) die("Bind socket for sender error");
	
	/* bind socket for receiver */
	memset((char *)&agent.receiver.addr, 0, sizeof(agent.receiver.addr));
	agent.receiver.addr.sin_family = AF_INET;
	agent.receiver.addr.sin_port = htons(AGENT_PORT_FOR_RECEIVER);
	agent.receiver.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(agent.receiver.fd, (struct sockaddr*) &agent.receiver.addr, sizeof(agent.receiver.addr)) < 0) die("Bind socket for sender error");

	struct sockaddr_in senderAddr, receiverAddr;
	senderAddr.sin_family = AF_INET;
	receiverAddr.sin_family = AF_INET;
	int senderAddr_len = sizeof(senderAddr);
	int receiverAddr_len = sizeof(receiverAddr);

	/* socket io multiplexing */
	fd_set read_fds, readMaster;
	fd_set write_fds, writeMaster;
	FD_ZERO(&readMaster);
	FD_ZERO(&writeMaster);
	FD_SET(agent.sender.fd, &readMaster);
	FD_SET(agent.receiver.fd, &readMaster);
	int max_fd = (agent.sender.fd > agent.receiver.fd)? agent.sender.fd: agent.receiver.fd;

	char sdr_pkt[PKT_LEN], recv_pkt[PKT_LEN];
	int sdr_len, recv_len;
	Header sdr_h, recv_h;
	char sdr_data[BUF_LEN], recv_data[BUF_LEN];
	while (1) {
		read_fds = readMaster;
		write_fds = writeMaster;			

		if (select(max_fd+1, &read_fds, &write_fds, NULL, NULL) < 0) die("Select error");
		if (FD_ISSET(agent.sender.fd, &read_fds)) {
			if ( (sdr_len = recvfrom(agent.sender.fd, sdr_pkt, PKT_LEN, 0, (struct sockaddr*)&senderAddr, &senderAddr_len)) < 0) {
				die("Receive from sender error");
			}
			if (sdr_len > 0) {
				print_packet(sdr_pkt);
				extract_packet(&sdr_h, sdr_data, sdr_pkt);
				inet_pton(AF_INET, sdr_h.destIP, &(receiverAddr.sin_addr));
				receiverAddr.sin_port = htons(sdr_h.destPort);
				FD_SET(agent.receiver.fd, &writeMaster);
			}
		}
		if (FD_ISSET(agent.receiver.fd, &read_fds)) {
			if ( (recv_len = recvfrom(agent.receiver.fd, recv_pkt, PKT_LEN, 0, (struct sockaddr*)&receiverAddr, &receiverAddr_len)) < 0) {
				die("Receive from receiver error");
			}
			if (recv_len > 0) {
				print_packet(recv_pkt);
				FD_SET(agent.sender.fd, &writeMaster);
			}
		}
		if (FD_ISSET(agent.sender.fd, &write_fds)) {
			if (sendto(agent.sender.fd, recv_pkt, recv_len, 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr)) < 0) {
				die("Send to sender error");
			}
			FD_CLR(agent.sender.fd, &writeMaster);
		}
		if (FD_ISSET(agent.receiver.fd, &write_fds)) {
			if (sendto(agent.receiver.fd, sdr_pkt, sdr_len, 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr)) < 0) {
				die("Send to receiver error");
			}
			FD_CLR(agent.receiver.fd, &writeMaster);
		}
	}

	close(agent.sender.fd);
	close(agent.receiver.fd);
	return 0;
}
