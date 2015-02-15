#include<stdio.h>
#include<netdb.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>

#define PORT "8000"
#define BACKLOG 10
#define MAX_COUNT 1024

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if(p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	if(listen(sockfd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connection...\n");

	while(1){
		sin_size = sizeof their_addr;
		if((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n",s);

		char filename[] = "hello.txt";
		int file_fd = open(filename, O_RDONLY);

		if(!fork()){ // child
			int msgSend;

			close(sockfd);
			while((msgSend = sendfile(new_fd, file_fd, NULL, MAX_COUNT)) != 0)
				if(msgSend == -1)
					perror("send");
			close(file_fd);
			close(new_fd);
			exit(0);
		}
		// parent
		close(new_fd);
		close(file_fd);
	}
	return 0;
}
