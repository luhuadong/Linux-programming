/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：simple_getopt.c
*   创 建 者：luhuadong
*   创建日期：2018年03月30日
*   描    述：使用getopt()的简单示例
*             -n —— 显示“我的名字”
*             -g —— 显示“她的名字”
*             -l —— 带参数的选项
*
================================================================*/


#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int oc;          // 选项字符
	char ec;         // 无效的选项字符
	char *b_opt_arg; // 选项参数字符串

	//opterr = 0;      // 当getopt()函数发现错误是强制不输出任何消息

	/* 如果optstring的第一个字符是冒号，则getopt()函数保持沉默，并在
	 * 出现“无效选项”时返回'?'，此时optopt包含了无效选项字符；在出现
	 * “缺少选项参数”时返回':'。如果optstring的第一个字符不是冒号，
	 * 那么不管是“无效选项”还是“缺少选项参数”，getopt()都会返回'?'。
	 */
	while((oc = getopt(argc, argv, ":ngl::")) != -1) {
	
		switch(oc) {
			
			case 'n':
				printf("My name is Rudy.\n");
				break;
			case 'g':
				printf("Her name is Tina.\n");
				break;
			case 'l':
				b_opt_arg = optarg;
				printf("Our love is %s.\n", optarg);
				break;
			case '?':
				ec = (char)optopt;
				printf("无效的选项字符'%c'\n", ec);
				break;
			case ':':
				printf("缺少选项参数\n");
				break;
			default:
				break;
		}
	}

	return 0;
}
