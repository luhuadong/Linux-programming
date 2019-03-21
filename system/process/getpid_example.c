/*================================================================
*   Copyright (C) 2019 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：getpid_example.c
*   创 建 者：luhuadong
*   创建日期：2019年03月21日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	printf("The current program's pid is %d\n", getpid());

	return 0;
}
