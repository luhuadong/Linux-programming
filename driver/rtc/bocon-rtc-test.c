/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   bocon-rtc-test.c
*   Author：     luhuadong
*   Create Date：2021年03月08日
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

#define TIME_LEN            7
#define BOCON_RTC_DEVICE    "/dev/bocon-rtc"

static char time_print[TIME_LEN][20]={

    "seconds",
    "minutes",
    "hours  ",
    "days   ",
    "months ",
    "years  ",
    "weeks  "
};

int main(int argc, char *argv[])
{
    int i, fd, ret, irqcount = 0;
    int data[TIME_LEN]={0};
    struct rtc_time t;

    fd = open(BOCON_RTC_DEVICE, O_RDWR);
    if(fd < 0)
    {
        printf("open %s failed\n", BOCON_RTC_DEVICE);
        return -1;
    }

    /* read */

    printf("get RTC time\n");
    ret = read(fd, data, sizeof(int) * TIME_LEN);
    if (ret == -1)
    {
        printf("read %s failed\n", BOCON_RTC_DEVICE);
    }

    for (i=0; i<TIME_LEN; i++)
    {
        printf("%s: %d\n", time_print[i], data[i]);
    }

    /* write */

    data[1] = data[1] + 10 < 60 ? data[1] + 10 : data[1] + 10 - 60;

    printf("set RTC time\n");
    ret = write(fd, data, sizeof(int) * TIME_LEN);
    if (ret == -1)
    {
        printf("write %s failed\n", BOCON_RTC_DEVICE);
    }

    /* read again */
    printf("get RTC time again\n");
    ret = read(fd, data, sizeof(int) * TIME_LEN);
    if (ret == -1)
    {
        printf("read %s failed\n", BOCON_RTC_DEVICE);
    }

    for (i=0; i<TIME_LEN; i++)
    {
        printf("%s: %d\n", time_print[i], data[i]);
    }

    /* close */

    ret = close(fd);
    if(ret < 0)
    {
        printf("close %s failed\n", BOCON_RTC_DEVICE);
    }
    
    return 0;
}
