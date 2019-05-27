/*================================================================
*   Copyright (C) 2019 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：hello.c
*   创 建 者：luhuadong
*   创建日期：2019年05月26日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	printf("Hello, %s!\n", getenv("USER"));

	return 0;
}
