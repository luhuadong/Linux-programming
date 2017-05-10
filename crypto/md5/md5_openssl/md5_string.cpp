#include <openssl/md5.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
{
	MD5_CTX c;
	unsigned char md[16];
	char *ss = "";
	char tmp[3]={'\0'}, md5buf[33]={'\0'};
	
	if(argc > 2) {
		printf("usage: ./main string\n");
		return -1;
	}
	else if(argc == 2) {
		ss = strdup(argv[1]);
		printf("string = %s\n", ss);
	}

	MD5_Init(&c);
	MD5_Update(&c, ss, strlen(ss));
	MD5_Final(md, &c);

	for(int i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02x", md[i]);
		strcat(md5buf, tmp);
	}

	cout << md5buf << endl;
	
	return 0;
}
