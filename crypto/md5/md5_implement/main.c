#include "md5.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i;
	md5_state_t s;
	char *ss = "";
	unsigned char result[17] = {0};

	if(argc > 2) {
		printf("usage: ./main string\n");
	}
	else if(argc == 2) {
		ss = strdup(argv[1]);
		printf("string = %s\n", ss);
	}

	md5_init(&s);
	md5_append(&s, (const unsigned char *)ss, strlen(ss));
	md5_finish(&s, (unsigned char *)result);

	for(i=0; i<16; i++)
	{
		printf("%x%x", (result[i]>>4)&(char)0x0f, result[i]&(char)0x0f);
	}
	printf("\n");

	//空字符串的MD5值是：d41d8cd98f00b204e9800998ecf8427e

	return 0;
}
