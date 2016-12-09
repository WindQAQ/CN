#include <stdio.h>
#include <string.h>
#include <errno.h>

void die(char* s)
{
	perror(s);
	exit(1);
}

int find_arg(int argc, char *argv[], char *s)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(s, argv[i])) {
			if (i == argc-1) {
				fprintf(stderr, "No argument given for %s\n", s);
				exit(1);
			}
			return i;
		}
	}
	return -1;
}
