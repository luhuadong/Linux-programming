/*================================================================
*   Copyright (C) 2017 Guangzhou GYT Ltd. All rights reserved.
*   
*   文件名称：unixstr_cli.c
*   创 建 者：luhuadong
*   创建日期：2017年10月23日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIXSTR_PATH "/tmp/unix.str"
#define LISTENQ 5
#define BUFFER_SIZE 256

int main(void)
{
	int sockfd;
	struct sockaddr_un servaddr;

	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSTR_PATH);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	char buf[BUFFER_SIZE];

	while(1)
	{
		bzero(buf, sizeof(BUFFER_SIZE));
		printf(">> ");
		if(fgets(buf, BUFFER_SIZE, stdin) == NULL)
		{
			break;
		}
		write(sockfd, buf, strlen(buf));
	}

	close(sockfd);
	
	return 0;
}
