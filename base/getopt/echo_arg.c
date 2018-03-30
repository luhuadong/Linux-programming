/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：echo_arg.c
*   创 建 者：luhuadong
*   创建日期：2018年03月30日
*   描    述：一个简单的命令行参数处理示例，它将参数输出到标准
*             设备上，用空格符隔开，最后换行。如果命令行第一个
*             参数为"-n"，则不会换行。
*             相比于getopt()等函数，该echo程序实现的是命令行参
*             数的手动解析。
*
================================================================*/

#include <stdio.h>

int main(int argc, char **argv)
{
	int i, nflg;
	printf("argc: %d\n", argc);
	
	nflg = 0;
	if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n') {
		nflg++;
		argc--;
		argv++;
	}

	for(i=1; i<argc; i++) {
		fputs(argv[i], stdout);
		if(i < argc-1) {
			putchar(' ');
		}
	}

	if(nflg == 0) {
		putchar('\n');
	}

	return 0;
}
