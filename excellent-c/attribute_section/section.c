/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：section.c
*   创 建 者：luhuadong
*   创建日期：2018年04月14日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

typedef void (*init_call)(void);

/* These two variables are defined in link script
 * 
 * 变量 _init_start 和 _init_end 在链接脚本中定义
 */ 
extern init_call _init_start;
extern init_call _init_end;

#define _init __attribute__((unused, section(".myinit")))
#define DECLARE_INIT(func) init_call _fn_##func _init = func

static void A_init(void)
{
	write(1, "A_init\n", sizeof("A_init\n"));
}

// 声明模块初始化函数
DECLARE_INIT(A_init);

static void B_init(void)
{
	printf("B_init\n");
}

DECLARE_INIT(B_init);

static void C_init(void)
{
	printf("C_init\n");
}

DECLARE_INIT(C_init);

/*
 * DECLARE_INIT like below:
 *
 * static init_call _fn_A_init __attribute__((unused, section(".myinit"))) = A_init;
 * static init_call _fn_B_init __attribute__((unused, section(".myinit"))) = B_init;
 * static init_call _fn_C_init __attribute__((unused, section(".myinit"))) = C_init;
 *
 */

void do_initcalls(void)
{
	init_call *init_ptr = &_init_start;

	for(; init_ptr < &_init_end; init_ptr++) {
	
		printf("init address: %p\n", init_ptr);
		(*init_ptr)();
	}
}

int main(void)
{
	do_initcalls();

	return 0;
}

