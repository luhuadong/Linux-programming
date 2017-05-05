#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("[%d]: before fork() ...\n", (int)getpid());

	pid_t x;
	x = fork();

	printf("[%d]: after fork() ...\n", (int)getpid());

	return 0;
}
