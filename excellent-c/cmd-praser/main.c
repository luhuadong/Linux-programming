/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：main.c
*   创 建 者：luhuadong
*   创建日期：2018年04月14日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <string.h>
#include "cmd.h"

extern void led_init(void);

void uart_get_string(char *str)
{
	char *find;

	printf(">> ");
	fgets(str, MAX_CMD_NAME_LENGTH, stdin);

	// Get rid of the newline character
	find = strchr(str, '\n');
	if(find) {
		*find = '\0';
	}
}

int main(void)
{
	char str[20];

	//uart_init();
	led_init();
	//beep_init();
	
	show_cmds();

	while(1)
	{
		uart_get_string(str);
		printf("recv: %s\n", str);

		match_cmd(str);

		//uart_send_string(str);
		//uart_send_byte('\n');
	}
}
