#include<stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVERPORT "4950"

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc != 2) {
		fprintf(stderr,"usage: %s message\n", argv[0]);
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("sender: socket");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "sender: failed to bind socket\n");
		return 2;
	}

	if((numbytes = sendto(sockfd, argv[1], strlen(argv[1]), 0, p->ai_addr, p->ai_addrlen)) == -1){
		perror("sender: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("sender: sent %d bytes to 127.0.0.1\n", numbytes);
	close(sockfd);
	return 0;
}
