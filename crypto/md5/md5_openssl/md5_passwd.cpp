#include <openssl/md5.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;

int main()
{
	string data = "helloworld";
	unsigned char md[16];
	int i;
	char tmp[3]={'\0'}, md5buf[33]={'\0'};

	MD5((unsigned char *)data.c_str(),data.length(),md);

	for( int  i=0; i<16; i++ ){
		sprintf(tmp,"%02x",md[i]);
		strcat(md5buf,tmp);
	}
	cout << md5buf << endl;

	return 0;
}
