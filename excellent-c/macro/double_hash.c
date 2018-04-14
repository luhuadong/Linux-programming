/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：double_hash.c
*   创 建 者：luhuadong
*   创建日期：2018年04月14日
*   描    述：展示C语言宏定义中的#和##的作用，其中单井表示将后面的
*	          宏参数作为字符串处理，也就是将后面的参数用双引号引起
*	          来，而双井号则用于连接。
*
================================================================*/


#include <stdio.h>

void quit_command()
{
	printf("I am quit command\n");
}

void help_command() 
{
	printf("I am help command\n");
}

struct command
{
	char *name;
	void (*function) (void);
};


#define COMMAND(NAME) {#NAME, NAME##_command}
#define PRINT(NAME) printf("token"#NAME"=%d\n", token##NAME)

int main(void)
{
	int token9 = 9;

	/*
	 * PRINT(9) 展开为：printf("token"#9"=%d\n", token##9)
	 * 其中：#9 即为 "9"，token##9 即为 token9
	 * 所以实际上就是：printf("token""9""=%d\n", token9)
	 * 输出：token9=9
	 */ 
	PRINT(9);
	//printf("token""9""=%d\n", token9);

	struct command commands[] = {
	
		COMMAND(quit),
		COMMAND(help),
	};
	commands[0].function();
	commands[1].function();

	return 0;
}

