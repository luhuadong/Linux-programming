/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：Daemon_exp.c
*   创 建 者：luhuadong
*   创建日期：2018年08月10日
*   描    述：
*
================================================================*/


#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>


int init_daemon(const char *pname, int facility)
{
	int pid;
	int i;

	// 处理可能的终端信号
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGHUP , SIG_IGN);

	// 创建子进程，父进程退出
	if(pid = fork()) {
		exit(EXIT_SUCCESS);
	}
	else if(pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	// 设置新会话组长，新进程组长，脱离终端
	setsid();

	// 创建新进程，子进程不能再申请终端
	if(pid = fork()) {
		exit(EXIT_SUCCESS);
	}
	else if(pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	// 关闭父进程打开的文件描述符
	for(i=0; i<NOFILE; ++i) {
		close(i);
	}
	
	// 标准输入输出全部重定向到 /dev/null
	// 因为先前关闭了所有的文件描述符，新开的值为 0,1,2
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	// 修改主目录
	chdir("/tmp");
	// 重新设置文件掩码
	umask(0);
	// 处理子进程退出
	signal(SIGCHLD, SIG_IGN);

	// 与守护进程建立联系，加上进程号，文件名
	openlog(pname, LOG_PID, facility);

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	time_t ticks;

	// 执行守护进程函数
	init_daemon(argv[0], LOG_KERN);

	while(1)
	{
		sleep(1);
		// 读取当前时间
		ticks = time(NULL);
		// 写日志信息
		syslog(LOG_INFO, "%s", asctime(localtime(&ticks)));
	}
	return 0;
}

