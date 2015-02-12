#include<stdio.h>
#include<errno.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define MYPORT "4950"
#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{
	int rv;
	int sockfd;
	int numbytes;
	socklen_t addr_len;
	char buf[MAXBUFLEN];
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("receiver: socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("receiver: bind");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "receiver failed: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("receiver: waiting for message...\n");

	addr_len = sizeof their_addr;
	if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
		perror("recvfrom");
		exit(1);
	}

	printf("receiver: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));

	printf("receiver: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("receiver: packet conatins \"%s\"\n", buf);

	close(sockfd);

	return 0;
}
