#include <stdio.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
  int i, fd, retval, irqcount = 0;
  unsigned long tmp, data;
  struct rtc_time rtc_tm;

  // 打开RTC设备
  fd = open ("/dev/rtc", O_RDONLY);

  if (fd ==  -1) {
    perror("/dev/rtc");
    exit(errno);
  }
  
  fprintf(stderr, "\n\t\t\tEnjoy TV while boiling water.\n\n");
  
  // 首先是一个报警器的例子，设定10分钟后"响铃"
  // 获取RTC中保存的当前日期时间信息
  /* Read the RTC time/date */
  retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
  if (retval == -1) {
    perror("ioctl");
    exit(errno);
  }

  fprintf(stderr, "\n\nCurrent RTC date/time is %d-%d-%d,%02d:%02d:%02d.\n", 
    rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
    rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

  // 设定时间的时候要避免溢出
  rtc_tm.tm_min += 10;
  if (rtc_tm.tm_sec >= 60) {
    rtc_tm.tm_sec %= 60;
    rtc_tm.tm_min++;
  }
  if  (rtc_tm.tm_min == 60) {
    rtc_tm.tm_min = 0;
    rtc_tm.tm_hour++;
  }
  if  (rtc_tm.tm_hour == 24)
    rtc_tm.tm_hour = 0;

  // 实际的设定工作
  retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
  if (retval == -1) {
    perror("ioctl");
    exit(errno);
  }

  // 检查一下，看看是否设定成功
  /* Read the current alarm settings */
  retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
  if (retval == -1) {
    perror("ioctl");
    exit(errno);
  }

  fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n",
   rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

  // 光设定还不成，还要启用alarm类型的中断才行(some RTC not support!)
  /* Enable alarm interrupts */
#if 1
  retval = ioctl(fd, RTC_AIE_ON, 0);
  if (retval == -1) {
    perror("ioctl");
    exit(errno);
  }
#endif
  
  // 现在程序可以耐心的休眠了，10分钟后中断到来的时候它就会被唤醒
  /* This blocks until the alarm ring causes an interrupt */
  retval = read(fd, &data, sizeof(unsigned long));
  if (retval == -1) {
    perror("read");
    exit(errno);
  }
  irqcount++;
  fprintf(stderr, " okay. Alarm rang.\n");
}