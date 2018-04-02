/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：echo_client.c
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

char wbuf[50];

int main()
{
	int sockfd;
	int size, on = 1;
	struct sockaddr_in saddr;
	struct sockaddr_in raddr;
	int ret;

	size = sizeof(struct sockaddr_in);
	bzero(&saddr, size);
	bzero(&raddr, size);

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.s_addr = inet_addr("120.78.197.79");

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("failed socket");
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int val = sizeof(struct sockaddr);

	while(1) {
	
		puts("please enter data: ");
		//gets(wbuf);
		fgets(wbuf, sizeof(wbuf), stdin);
		ret = sendto(sockfd, wbuf, 50, 0, (struct sockaddr*)&saddr, sizeof(struct sockaddr));
		if(ret < 0) {
			perror("sendto failed");
		}

		bzero(wbuf, 50);

		ret = recvfrom(sockfd, wbuf, 50, 0, (struct sockaddr*)&raddr, &val);
		if(ret < 0) {
			perror("recvfrom failed");
		}
		printf("ip: %s, port: %d\n", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
		printf("the data : %s\n", wbuf);

		bzero(wbuf, 50);
	}

	close(sockfd);
	return 0;
}
