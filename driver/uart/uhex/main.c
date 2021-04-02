//#include "../lib/serial/serial.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

int AscToHex(char hexstr[], char **out);
char CheckSum(char *buf, int len);

typedef struct _usartAttr
{
	int speed;
	int bits;
	int event;
	int stop;
	int fd;
	int write_timeout;
	int read_timeout;
	int serialport;
}UsartAttr;

void showUsage()
{
	puts("Options:" );
	puts(" -p, --serialport=portnum   The serial port to send." );
	puts(" -c, --checksum             If add checksum at the end." );
	puts(" -B, --baud=speed           The serial speed eg.115200 9600 4800" );
	puts(" -v, --version              Print the version number and exit." );
	puts(" -h, --help                 Print this message and exit." );
	puts(" --buf=senddate             The buf which need to send eg. " "ff 01 00 25  " "" );
	exit(0);
}

int lopt;
static struct option longOpts[] = {
	{ "serial-port", 	required_argument, 	NULL,  'p' },
	{ "checksum",	 	no_argument,	    NULL,  'c' },
	{ "baud",		 	required_argument, 	NULL,  'B' },
	{ "version",	 	no_argument,	    NULL,  'v' },
	{ "help",	 		no_argument,	    NULL,  'h' },
	{ "buf",	 		required_argument, &lopt, 'b' },
	{ 0,		 0,		    0,	   0   }
};

int main(char argc, char *argv[])
{
	char flag_checksum = 0;
	char *sendbuf = NULL;
	int buflen = 0;
	UsartAttr SerialAttr = { 115200, 8, 'N', 1, 0, 500, 500, 1 };

	while (1)
	{
		int optIndex = 0;
		char c = getopt_long(argc, argv, "p:b:cB:vh", longOpts, &optIndex);
		if ( c == '?' || c == -1)
		{
			showUsage();
		}
		if (c == 0xff)
		{
			if (sendbuf == NULL)
			{
				showUsage();
			}
			break;
		}
		switch (c)
		{
			case 0:
			{
				switch (lopt)
				{
				case 'b':
				{

					buflen = AscToHex(optarg, &sendbuf);
					if (buflen == -1)
					{
						printf("--buf  error \n");
						goto cleanup;
					}
					break;
				}
				}
				break;
			}
			case 'p':
				SerialAttr.serialport = *optarg - '0';
				break;
			case 'c':
				flag_checksum = 1;
				break;
			case 'B':
				SerialAttr.speed = atoi(optarg);
				break;
			case 'v':
				printf("version: V1.0\n");
				break;
			case 'h':
				showUsage();
				break;
			default:
				break;
		}

	}

	UsartOpenAPI(&SerialAttr);
	UsartWriteAPI(&SerialAttr, sendbuf, buflen);
	if (flag_checksum == 1)
	{
		char sum = CheckSum(sendbuf + 1, buflen - 1);
		printf("Send check sum : ");
		UsartWriteAPI(&SerialAttr, &sum, 1);
	}
	
	UsartCloseAPI(&SerialAttr);
cleanup:
	free(sendbuf);
	return 0;
}

char CheckSum(char *buf, int len)
{
	char ret = 0;

	for (int i = 0; i < len; i++)
	{
		ret += buf[i];
	}
	return ret;
}

char ChToNum(char ch)
{
	char ret = 0;

	if (ch >= '0' && ch <= '9')
	{
		ret = ch - '0';
	}
	else if (ch >= 'a' && ch <= 'f')
	{
		ret = ch - 'a' + 10;
	}
	else if (ch >= 'A' && ch <= 'F')
	{
		ret = ch - 'A' + 10;
	}
	return ret;
}

int AscToHex(char hexstr[], char **out)
{
	int len = strlen(hexstr);
	int i = 0, j = 0, k = 0;

	k = len;
	while (k--)
	{
		if (!((hexstr[k] == ' ') ||
		      (hexstr[k] >= '0' && hexstr[k] <= '9' ) ||
		      (hexstr[k] >= 'a' && hexstr[k] <= 'f' ) ||
		      (hexstr[k] >= 'A' && hexstr[k] <= 'F' )))
		{
			return -1;
		}
	}
	char *hexstr2 = (char *)malloc(len);
	while (i < len)
	{
		(hexstr[i] != ' ') ? (hexstr2[j++] = hexstr[i++]) : i++;
	}
	hexstr2[j--] = '\0';
	*out = (char *)malloc(j / 2);
	for (i = 0; i < j; i += 2)
	{
		(*out)[i / 2] = ((ChToNum(hexstr2[i])) << 4) + (ChToNum(hexstr2[i + 1]));
	}
	free(hexstr2);
	return j / 2 + 1;
}

