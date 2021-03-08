#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int main()
{
    int fd, ret;
    struct rtc_time rtc_tm;
    unsigned long data;

    printf("======RTC Test ====\n");

    printf("Open & release\n");
    //open function is used to establish the connection between the RTC device with the file descriptor
    
    fd=open("/dev/rtc0", O_RDWR, 0);      //open for reading & writing
    if (fd==-1)
    {
        perror("/dev/rtc0");
        exit(errno);
    }
    else
    {
        printf("opened\n");
    }

    //ioctl() to configure the RTC device
    printf("get RTC time\n");
    ret=ioctl(fd, RTC_RD_TIME, &rtc_tm);  //ioctl command RTC_RD_TIME is used to read the current timer
    if (ret==-1)
    {
        perror("rtc ioctl RTC_RD_TIME error");
    }
    printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d\n",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

        /* Set the RTC time/date */
       /* rtc_tm.tm_mday=31;
        rtc_tm.tm_mon=4;      // for example Sep 8
        rtc_tm.tm_year=104;
        rtc_tm.tm_hour=2;
        rtc_tm.tm_min=30;
        rtc_tm.tm_sec=0;
        */

    // call the read function to wait the Alarm interrupt
    printf("read RTC time\n");
    ret=read(fd, &data, sizeof(unsigned long));
    if (ret==-1)
    {
        perror("rtc read error");
    }
    printf("rtc read\n");

    // set RTC time
    printf("set RTC time\n");
    
    ret==ioctl(fd, RTC_SET_TIME, &rtc_tm);
    if (ret==-1)
    {
        perror("rtc ioctl RTC_SET_TIME error");
    }
    printf("Set Current RTC date/time to %d-%d-%d, %02d:%02d:%02d\n",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
    printf("Get RTC time\n");
    
    ret=ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (ret==-1)
    {
        perror("rtc ioctl RTC_RD_TIME error");
    }
    printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d\n",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    rtc_tm.tm_sec +=50;
    if (rtc_tm.tm_sec >=60)
    {
        rtc_tm.tm_sec %=60;
        rtc_tm.tm_min++;
    }
    if (rtc_tm.tm_min==60)
    {
        rtc_tm.tm_min=0;
        rtc_tm.tm_hour++;
    }
    if (rtc_tm.tm_hour==24)
    {
        rtc_tm.tm_hour=0;
    }
    printf("RTC tests done");
    close(fd);

    return 0;
}