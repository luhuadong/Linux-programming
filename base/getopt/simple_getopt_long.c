/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：simple_getopt_long.c
*   创 建 者：luhuadong
*   创建日期：2018年03月30日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <getopt.h>

int do_name, do_gf_name;
char *l_opt_arg;

#if 0
struct option longopts[] = {
	{"name", no_argument, &do_name, 1},
	{"gf_name", no_argument, &do_gf_name, 1},
	{"love", required_argument, NULL, 'l'},
	{0, 0, 0, 0}
};
#else
struct option longopts[] = {
	{"name",    no_argument,       NULL, 'n'},
	{"gf_name", no_argument,       NULL, 'g'},
	{"love",    required_argument, NULL, 'l'},
	{0, 0, 0, 0}
};
#endif

int main(int argc, char **argv)
{
	int c;

	while((c = getopt_long(argc, argv, ":l:", longopts, NULL)) != -1) {
		
		switch(c) {
#if 0
			case 'l':
				l_opt_arg = optarg;
				printf("Our love is %s!\n", l_opt_arg);
				break;
			case 0:
				printf("getopt_long() 设置变量 : do_name = %d\n", do_name);
				printf("getopt_long() 设置变量 : do_gf_name = %d\n", do_gf_name);
				break;
#else
			case 'n':
				printf("My name is Rudy.\n");
				break;
			case 'g':
				printf("Her name is Tina.\n");
				break;
			case 'l':
				l_opt_arg = optarg;
				printf("Our love is %s!\n", l_opt_arg);
				break;
#endif
		}
	}
	return 0;
}
