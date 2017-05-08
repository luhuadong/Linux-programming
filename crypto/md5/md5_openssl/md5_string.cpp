#include <openssl/md5.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
{
	FILE *fd=fopen("tmp.txt","r");
	MD5_CTX c;
	unsigned char md[16];
	int len;
	char tmp[3]={'\0'}, md5buf[33]={'\0'};
	unsigned char buffer [1024]={'\0'};

	MD5_Init(&c);

	while( 0 != (len = fread(buffer, 1, 1024, fd) ) )
	{
		MD5_Update(&c, buffer, len);
	}

	MD5_Final(md,&c);

	for(int i = 0; i < 16; i++)
	{
		sprintf(tmp,"%02X",md[i]);
		strcat(md5buf,tmp);
	}
	cout<<md5buf<<endl;
	fclose(fd);

	return 0;
}
