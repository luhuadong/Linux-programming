/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   rtc-read.c
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

#define TIME_LEN   7

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
    struct rtc_time t;

    int data[TIME_LEN]={0};

    fd = open("/dev/rtc", O_RDWR);
    if(fd < 0)
    {
        printf("open file : %s failed !\n", argv[0]);
        return -1;
    }

    /* ioctl command RTC_RD_TIME is used to read the current timer */
    printf("get RTC time\n");
    ret = ioctl(fd, RTC_RD_TIME, &t);
    if (ret==-1)
    {
        perror("rtc ioctl RTC_RD_TIME error");
    }
    printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d, %d\n",
            t.tm_mday, t.tm_mon + 1, t.tm_year,
            t.tm_hour, t.tm_min, t.tm_sec, t.tm_wday);

    data[0] = t.tm_sec;
    data[1] = t.tm_min;
    data[2] = t.tm_hour;
    data[3] = t.tm_mday;
    data[4] = t.tm_mon + 1;
    data[5] = t.tm_year + 1900;
    data[6] = t.tm_wday;

    for (i=0; i<TIME_LEN; i++)
    {
        printf("%s: %d\n", time_print[i], data[i]);
    }

    ret = close(fd);
    if(ret < 0)
    {
        printf("close file error! \n");
    }
    
    return 0;
}
