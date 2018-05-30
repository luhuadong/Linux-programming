/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：mtd_read.c
*   创 建 者：luhuadong
*   创建日期：2018年05月18日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <errno.h>
#include <getopt.h>

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

#define BUFFER_SIZE		1024
#define OTP_NUM_WORDS	0x40

#define SUCCESS		0
#define FAIL		-1
#define TRUE		1
#define FALSE		0

static const char * const short_options = "hd:r:w::e";

static const struct option long_options[] = {

	{"help",   no_argument,       NULL, 'h'},
	{"device", required_argument, NULL, 'd'},
	{"read",   required_argument, NULL, 'r'},
	{"write",  optional_argument, NULL, 'w'},
	{"erase",  no_argument,       NULL, 'e'},
	{0, 0, 0, 0}
};

typedef enum {

	RUN_READ,
	RUN_WRITE,
	RUN_ERASE,
	RUN_END
} run_type;

/*
struct mtd_info_user {
	__u8 type;
	__u32 flags;
	__u32 size;		// Total size of the MTD
	__u32 erasesize;
	__u32 writesize;
	__u32 oobsize;	// Amount of OOB data per block (e.g. 16)
	__u64 padding;	// Old obsolete field; do not use
};
*/

static struct mtd_info_user info;

static void show_usage()
{
	printf("\nUsage: ./mtd_go -d <device> <operation>\n");
	printf("\nOperation:\n");
	printf("\t-r\n");
	printf("\t-w\n");
	printf("\t-e\n");

	exit(0);
}

static int mtd_read(const char *device, const unsigned int size)
{
	assert(device);
	assert(size > 0);

	// open the device
	int fd, ret = -1;
	fd = open(device, O_RDONLY);
	if(fd < 0) {
		printf("(E) Open device %s failed.\n", device);
		return -1;
	}
	
	char buf[size];
	memset(buf, 0, size);

	ret = read(fd, buf, size);
	if(ret != -1) {
		printf("buffer_read: \n");
		int i;
		for(i = 0; i < ret; i++) {

			printf("%02x ", buf[i]);
			if(!((i+1)%16)) printf("\n");
		}
		printf("\n Read ok!\n");
	}
	else {
		printf("(E) Read error!\n");
	}

	close(fd);
	return ret;
}


static int mtd_write(const char *device, const void *data, const unsigned int size)
{
	assert(device);
	assert(data);
	assert(size > 0);

	// open the device
	int fd, ret = -1;
	fd = open(device, O_RDWR);
	if(fd < 0) {
		printf("(E) Open device %s failed.\n", device);
		return -1;
	}
	
	ret = write(fd, data, size);
	if(ret != -1) {
		printf("\n Write ok!\n");
	}
	else {
		printf("(E) Write error!\n");
	}

	close(fd);
	return ret;
}


static int non_region_erase(const int fd, const int start, int count, const int unlock)
{

	mtd_info_t meminfo;

	if(ioctl(fd, MEMGETINFO, &meminfo) == 0) {

		erase_info_t erase;
		erase.start = start;
		erase.length = meminfo.erasesize;

		for(; count > 0; count--) {

			printf("Performing Flash Erase of length %u at offset 0x%x\n", erase.length, erase.start);
			fflush(stdout);

			if(unlock != 0) {

				printf("Performing Flash unlock at offset 0x%x\n", erase.start);
				if(ioctl(fd, MEMUNLOCK, &erase) != 0) {
				
					perror("MTD unlock failure");
					close(fd);
					return -1;
				}
			}

			if(ioctl(fd, MEMERASE, &erase) != 0) {
			
				perror("MTD erase failure");
				close(fd);
				return -1;
			}
			erase.start += meminfo.erasesize;
		}
		printf("  done\n");
	}
	return 0;
}

static int mtd_erase(const char *device, const unsigned int regcount)
{
	assert(device);

	// open the device
	int fd, ret = -1;
	fd = open(device, O_RDWR);
	if(fd < 0) {
		printf("(E) Open device %s failed.\n", device);
		return -1;
	}

	if(regcount == 0) {

		ret = non_region_erase(fd, 0, (info.size / info.erasesize), 0);
		if(ret == 0) {
			printf("\n Erase ok!\n");
		}
		else {
			printf("(E) Erase error!\n");
		}
	}

	close(fd);
	return ret;
}

static int EeReadLineFromEepFile(FILE* DataFile, s8 *Buffer, const u32 BufferSize)
{
  u32         Sign            = 0;
  u16         StringIndex     = 0;
  s16	      EeStatus        = SUCCESS;
  boolean     EndOfLine       = FALSE;
  boolean     Comment         = FALSE;

  do
  {
    /* Read line from the file. If the line is longer
     * it will be serviced below */
    if(fgets(Buffer, BufferSize, DataFile) == NULL)
    {
      EeStatus = FAIL;
      break;
    }

    /* Look for:
     * a semicolon in the string indicating a comment
     * a EOL just for detection if this is a full line
     * read to the buffer */
    for(StringIndex = 0; StringIndex < BufferSize; StringIndex++)
    {
      /* Break the loop if string has ended */
      if(Buffer[StringIndex] == '\0')
      {
        break;
      }

      /* Detect end of line and comments */
      if(Buffer[StringIndex] == '\n' ||
         Buffer[StringIndex] == '\r' ||
         Buffer[StringIndex] == ';')
      {
        /* Determine which we encountered...EOL or
 	 * comment...add terminating NULL as needed */
        if(Buffer[StringIndex] == ';')
        {
          Comment = TRUE;
          Buffer[StringIndex] = '\0';
        }
        else
        {
          EndOfLine = TRUE;
        }
      }
    }

    /* If the end of line was not detected - read the rest of
     * line and omit it if it's a comment */
    if(EndOfLine == FALSE)
    {
      if(Comment == TRUE)
      {
        while((Sign = getc(DataFile)) != EOF)
        {
          if(Sign == (u32)'\n' || Sign == (u32)'\r')
          {
            break;
          }
        }
      }
      /* If data in buffer does not contain the whole line
       * (it does not contain comment)
       * then it will be read in next step */
    }

    /* If this line is empty, clear local flags */
    if(*Buffer == '\0')
    {
      EndOfLine =  FALSE;
      Comment = FALSE;
    }
  }while(*Buffer == '\0');

  return EeStatus;
}

static int mtd_write_file(const char *device, const char *filename)
{
	s8	retval = FAIL;
	u16 i = 0;
	s16 maxlen = 64;
	FILE *file = NULL;
	u16 b[8];
	s8 temp[200];
	u16 buffer[OTP_NUM_WORDS];

	file = fopen(filename, "r");
	if(NULL == file) {
		printf("Unable to open specified file: %s\n", filename);
		return -1;
	} 
	else {
		FILE *file_ro = file; // WTF! (FILE *)0x12008 --> (FILE *)0x10000
		memset(b, 0x0, sizeof(u16)*8);
		memset(buffer, 0x0, sizeof(u16)*OTP_NUM_WORDS);

		retval = EeReadLineFromEepFile(file, temp, 200);

		while ((SUCCESS == retval) && (i < maxlen))
		{
			// place the hex numbers from the line read in to the temp buffer
			sscanf(temp, "%04x %04x %04x %04x %04x %04x %04x %04x", \
					&(b[0]), &(b[1]), &(b[2]), &(b[3]), \
					&(b[4]), &(b[5]), &(b[6]), &(b[7]));
			//printf("%04x %04x %04x %04x %04x %04x %04x %04x\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);

			// copy the words to the permanent buffer
			buffer[i+0] = b[0];
			buffer[i+1] = b[1];
			buffer[i+2] = b[2];
			buffer[i+3] = b[3];
			buffer[i+4] = b[4];
			buffer[i+5] = b[5];
			buffer[i+6] = b[6];
			buffer[i+7] = b[7];

			// increment i by 8 so we can get the next 8 words
			i += 8;

			// read the next line
			retval = EeReadLineFromEepFile(file, temp, 200);
		}
	}
	fclose(file);
	mtd_write(device, buffer, sizeof(buffer));

	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	char *device = NULL;
	int regcount;
	run_type mode = -1;
	int read_size = 0;
	char *opt_file = NULL;

	int c;
	while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {

		switch(c) {

			case 'h': show_usage();
					  break;
			case 'd': device = optarg;
					  break;
			case 'r': {
						  mode = RUN_READ;
						  read_size = atoi(optarg);
					  }
					  break;
			case 'w': {
						  mode = RUN_WRITE;
						  opt_file = optarg;
						  printf("opt_file : %s\n", opt_file);
					  }
					  break;
			case 'e': mode = RUN_ERASE;
					  break;
			default: break;
		}
	}

	if(device == NULL) {
		printf("(E) Device is required, please check it.\n");
		show_usage();
	}
	printf("device = %s\n", device);

	// open the device
	fd = open(device, O_RDWR);
	if(fd < 0) {
		printf("open device %s error\n", device);
		return -1;
	}
	
	if(ioctl(fd, MEMGETINFO, &info) == 0) {
	
		printf("info.size=%d\n", info.size);
		printf("info.erasesize=%d\n", info.erasesize);
		printf("info.writesize=%d\n", info.writesize);
		printf("info.oobsize=%d\n", info.oobsize);
	}

	if(ioctl(fd, MEMGETREGIONCOUNT, &regcount) == 0) {
		
		printf("regcount = %d\n", regcount);
	}

	close(fd);

	if(RUN_READ == mode) {
	
		// Read
		mtd_read(device, read_size);
	} 
	else if(RUN_WRITE == mode) {
	
		// Write
		if(opt_file) {
			printf("Write opt_file to device %s\n", device);
			mtd_write_file(device, opt_file);
		}
		else {
			printf("Write buf[8] to device %s\n", device);
			char buf[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
			mtd_write(device, buf, sizeof(buf));
		}
	} 
	else if(RUN_ERASE == mode) {
	
		// Erase
		mtd_erase(device, regcount);
	}

	return 0;
}
