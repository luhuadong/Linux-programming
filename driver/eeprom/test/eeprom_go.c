/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：eeprom_go.c
*   创 建 者：luhuadong
*   创建日期：2018年05月31日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define NDEBUG
#include <assert.h>

// Data types
#define u8		unsigned char 
#define u16		unsigned short
#define u32		unsigned long 
#define u64		unsigned long long int 

#define s8		char 
#define s16		int 
#define s32		long int 
#define s64		long long int 
#define boolean unsigned char

static int eeprom_read(const int fd, const u32 offset, void *dev_to_buf, const u32 size)
{
	lseek(fd, offset, SEEK_SET);
	return (read(fd, dev_to_buf, size));
}

static int eeprom_write(const int fd, const u32 offset, const void *buf_to_dev, const u32 size)
{
	lseek(fd, offset, SEEK_SET);
	return (write(fd, buf_to_dev, size));
}

int main(int argc, char *argv[])
{
	int fd, size, len, i;
	char buf[50] = {0};
	char *bufw = "Hi,this is an eeprom test.";

	if(argc > 1) 
		bufw = argv[1];

	len = strlen(bufw);

	// Open
	
	fd = open("/sys/bus/spi/drivers/at25/spi32764.0/eeprom", O_RDWR);
	if(fd < 0) {
		printf("(E) SPI EEPROM device open failed\n");
		return -1;
	}
	printf("(I) Open device OK\n");

	// Write
	size = eeprom_write(fd, 0x40, bufw, len);
	if(size < 0) {
		printf("(E) Write error\n");
	} else {
		printf("(I) Write OK : %s\n", bufw);
	}

	// Read
	size = eeprom_read(fd, 0x40, buf, len);
	if(size < 0) {
		printf("(E) Read error\n");
	} else {
		printf("(I) Read OK\n");
	}

	printf(">>\n");
	for(i=0; i<len; i++) {
		printf(" %02x", buf[i]);
		if(!((i+1)%16)) printf("\n");
	}
	printf("\nRead finish\n");

	close(fd);
	
	return 0;
}
