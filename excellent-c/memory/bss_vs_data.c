/*================================================================
*   Copyright (C) 2020 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：bss_vs_data.c
*   创 建 者：luhuadong
*   创建日期：2020年12月31日
*   描    述：比较BSS段和数据段的全局变量对可执行文件的影响，
*             BSS段不占用可执行文件空间，Data段则需要占用。
*
================================================================*/


#include <stdio.h>

#if 0
int ar[30000];                       /* BSS */
#else
int ar[30000] = {1, 2, 3, 4, 5, 6};  /* Data */
#endif

int main(void)
{
	printf("hello world\n");
	
	return 0;
}

