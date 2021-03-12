/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   gpio_test.c
*   Author：     luhuadong
*   Create Date：2021年03月12日
*   Description：
*
================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>

void usage(char *cmd)
{
    printf("Usage: %s </dev/gpio> [value]\n", cmd);
    printf("e.g. Read : %s /dev/gpio1\n", cmd);
    printf("e.g. Write: %s /dev/gpio1 0\n\n", cmd);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return -1;
    }

    char *device_file = argv[1];
    int ret, value, fd;

    fd = open(device_file, O_RDWR);
    if(fd < 0)
    {
        printf("open %s failed\n", device_file);
        return -1;
    }

    /* get value */
    if (argc == 2) {
        ret = read(fd, &value, sizeof(value));
        if (ret < 0)
        {
            printf("read %s failed\n", device_file);
        }
        else {
            printf("Get %s value %d\n", device_file, value);
        }
    }
    /* set value */
    else {
        value = atoi(argv[2]);
        if (value != 0 && value != 1) {
            printf("Invalid value , please set 0 or 1\n");
            close(fd);
            return -1;
        }
        ret = write(fd, &value, sizeof(value));
        if (ret < 0)
        {
            printf("write %s failed\n", device_file);
        }
        else {
            printf("Set %s value %d\n", device_file, value);
        }
    }

    ret = close(fd);
    if(ret < 0)
    {
        printf("close %s failed\n", device_file);
    }

    return 0;
}
