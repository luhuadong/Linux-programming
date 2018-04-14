/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：cmd.c
*   创 建 者：luhuadong
*   创建日期：2018年04月13日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <string.h>

#include "cmd.h"

//static xdata CMDS commands = {NULL, 0};
static CMDS commands = {0};

void show_cmds(void)
{
	int i;
	for(i=0; i<commands.num; i++) {
		printf("CMD: %s\n", commands.cmds[i].cmd_name);
	}
}

void match_cmd(char *str)
{
	int i;
	if(strlen(str) > MAX_CMD_NAME_LENGTH) {
		return ;
	}
	printf("matching ... ");

	for(i=0; i<commands.num; i++) {
		if(strcmp(commands.cmds[i].cmd_name, str) == 0) {
			printf("match ok\n");
			commands.cmds[i].cmd_operate();
			return ;
		}
	}
	printf("match failed\n");
}

void register_cmds(const CMD reg_cmds[], const int length)
{
	int i;
	if(length > MAX_CMDS_COUNT) {
		return ;
	}
	printf("register_cmds ...\n");

	for(i=0; i<length; i++) {
		if(commands.num < MAX_CMDS_COUNT) {
			strcpy(commands.cmds[commands.num].cmd_name, reg_cmds[i].cmd_name);
			commands.cmds[commands.num].cmd_operate = reg_cmds[i].cmd_operate;
			commands.num++;
		}
	}
}
