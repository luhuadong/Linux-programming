/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：cmd.h
*   创 建 者：luhuadong
*   创建日期：2018年04月13日
*   描    述：
*
================================================================*/


#pragma once

#define MAX_CMD_NAME_LENGTH	20
#define MAX_CMDS_COUNT		10

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

typedef void (*handler)(void);

typedef struct cmd {
	
	char cmd_name[MAX_CMD_NAME_LENGTH + 1];
	handler cmd_operate;
} CMD;

typedef struct cmds {
	
	CMD cmds[MAX_CMDS_COUNT];
	int num;
} CMDS;


void show_cmds(void);
void match_cmd(char *str);
void register_cmds(const CMD reg_cmds[], const int length);



