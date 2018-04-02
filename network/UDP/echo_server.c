/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：echo_server.c
*   创 建 者：luhuadong
*   创建日期：2018年04月02日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


char rbuf[50];

int main()
{
	int sockfd;
	int size;
	int ret;
	int on = 1;
	struct sockaddr_in saddr;
	struct sockaddr_in raddr;

	size = sizeof(struct sockaddr_in);
	bzero(&saddr, size);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("socket failed");
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	ret = bind(sockfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr));
	if(ret < 0) {
		perror("bind failed");
		return -1;
	}

	int val = sizeof(struct sockaddr);

	while(1) {
	
		puts("waiting data ......");
		ret = recvfrom(sockfd, rbuf, 50, 0, (struct sockaddr*)&raddr, &val);
		if(ret < 0) {
			perror("recvfrom failed");
		}
		printf("ip: %s, port: %d\n", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
		printf("the data : %s\n", rbuf);

		ret = sendto(sockfd, rbuf, strlen(rbuf), 0, (struct sockaddr*)&raddr, sizeof(struct sockaddr));
		if(ret < 0) {
			perror("send failed");
		} else {
			printf("send success.\n");
		}
		bzero(rbuf, 50);
	}

	close(sockfd);
	return 0;
}
