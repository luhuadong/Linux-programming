/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：main.c
*   创 建 者：luhuadong
*   创建日期：2018年04月03日
*   描    述：演示如何调用Dev中的设备接口
*
================================================================*/


#include "dev.h"

#define CNT_MAX 10

void DEV_Example(void)
{
	int i = 0;

	Dev_Init();

	for(i = 0; i < CNT_MAX; ++i) {

		Dev_PrintInt(i);
	}

	Dev_Close();
}

int main(void)
{
	DEV_Example();

	return 0;
}
