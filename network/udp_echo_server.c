/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：udp_echo_server.c
*   创 建 者：luhuadong
*   创建日期：2018年06月11日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUF (8*1024)

int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport;
	char buf[MAXBUF];

	if(argc != 3) {
	  printf("Error, please make sure your command.\n");
	  printf("Usage: ./udp_echo_server <ip> <port>\n\n");
	  exit(EXIT_FAILURE);
	}

	if(argv[2])
		myport = atoi(argv[2]);
	else
		myport = 7575;
	
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

    int optval = 1;
	int optlen = sizeof(optval);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(myport);

	if(argv[1]) {
		my_addr.sin_addr.s_addr = inet_addr(argv[1]);
	}
	else {
		my_addr.sin_addr.s_addr = INADDR_ANY;
	}

	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

// ok

    int datalen;
	len = sizeof(their_addr);
	while(1)
	{
		bzero(buf, MAXBUF);
		datalen = recvfrom(sockfd, buf, sizeof(buf), 0, 
		                   (struct sockaddr *)&their_addr, &len);
        if(datalen > 0) {
		    datalen = sendto(sockfd, buf, strlen(buf), 0, 
			                 (struct sockaddr *)&their_addr, len);
		}
	}

	close(sockfd);
	return 0;
}


