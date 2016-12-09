#include <sys/socket.h>

typedef struct socket_bindaddr {
	int fd;
	struct sockaddr_in addr;
} Socket_addr;

typedef struct agent {
	Socket_addr sender, receiver;
} Agent;
