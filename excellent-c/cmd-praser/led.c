/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：led.c
*   创 建 者：luhuadong
*   创建日期：2018年04月13日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <linux/kernel.h>
#include "cmd.h"

static void led_on(void)
{
	printf("LED ON\n");
}

static void led_off(void)
{
	printf("LED OFF\n");
}

static void led_toggle(void)
{
	printf("LED TOGGLE\n");
}

void led_init(void)
{
	CMD led_cmds[] = {
		{"led on",  led_on},
		{"led off", led_off},
		{"led toggle", led_toggle}
	};

	register_cmds(led_cmds, ARRAY_SIZE(led_cmds));

	led_off();
	printf("led_init finished\n");
}
