#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/tps65910.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/spinlock.h>

#include <linux/types.h>		/* For standard types (like size_t) */
#include <linux/errno.h>		/* For the -ENODEV/... values */
#include <linux/unistd.h>		

struct time_t{
    char seconds;
    char minutes;
    char hours;
    char days;
    char months;
    char years;
    char weeks;
};

#define TIME_LEN 7
#define YEAR_POS 5 /*0-6*/
union time_type{
    char buf[TIME_LEN];
    struct time_t time;
};

struct tps65910_rtc_dev {
     char *name;
     struct tps65910 *tps65910;
     struct class *rtc_class;
     struct device *dev;
     int major;
     union time_type tm;
     spinlock_t lock;
};

struct tps65910_rtc_dev rtc_dev={
    .name = "rtc-tps",
};

#define RTC_READ(reg,bytes,val)     rtc_dev.tps65910->read(rtc_dev.tps65910,reg,bytes,&val)
#define RTC_WRITE(reg,bytes,val)   rtc_dev.tps65910->write(rtc_dev.tps65910,reg,bytes,&val)

#define DBG_PRT printk
//#define DBG_PRT(arg..)
#if 0
int seconds_range[2]={0,59};
int minutes_range[2]={0,59};
int hours_range[2]={0,23};
int days_range[2]={1,31};
int months_range[2]={1,12};
int years_range[2]={2000,2099};
#else

static char time_print[TIME_LEN][20]={
    "seconds",
    "minutes",
    "hours",
    "days",
    "months",
    "years",
    "weeks"
};

static int time_range[TIME_LEN][2]={
    {0,59},/*seconds*/
    {0,59},/*minutes*/
    {0,23},/*hours*/
    {1,31},/*days*/
    {1,12},/*months*/
    {2000,2099},/*years*/
    {1,6},
};

static int days_range_leap[12][2]={
        {1,31},/*month : 1 */
        {1,29},/*2*/
        {1,31},/*3*/
        {1,30},/*4*/
        {1,31},/*5*/
        {1,30},/*6*/
        {1,31},/*7*/
        {1,31},/*8*/
        {1,30},/*9*/
        {1,31},/*10*/
        {1,30},/*11*/
        {1,31}/*12*/
};
static int days_range_common[12][2]={
        {1,31},/*month : 1 */
        {1,28},/*2*/
        {1,31},/*3*/
        {1,30},/*4*/
        {1,31},/*5*/
        {1,30},/*6*/
        {1,31},/*7*/
        {1,31},/*8*/
        {1,30},/*9*/
        {1,31},/*10*/
        {1,30},/*11*/
        {1,31}/*12*/
};

#endif

#include<linux/delay.h>
int rtc_open(struct inode *inode, struct file *flip)
{
    char val=0;
    
    spin_lock(&rtc_dev.lock);
    RTC_READ(TPS65910_RTC_CTRL,1,val);
    val  &= ~(BIT(0));/*stop rtc*/
    RTC_WRITE(TPS65910_RTC_CTRL,1,val);

    udelay(10);
    
    RTC_READ(TPS65910_DEVCTRL,1,val);
    val &= ~ BIT(6); /*RTC in normal power mode*/ /*device slip into BACKUP state for powering up rtc*/
    val &= ~ BIT(5);/*the internal 32-kHz clock source is the crystal oscillator or an 
    external 32-kHz clock in case the crystal oscillator is used in bypass 
    mode*/
    RTC_WRITE(TPS65910_DEVCTRL,1,val);
    
    //RTC_READ(TPS65910_DEVCTRL,1,val);
    // 0x7fff means that decrease 1 seconds, 0x8001 means that inscrease 1 seconds
    //val=0x556 // too fast 3 seconds, 9-24 9:40:10 TO 9-26 14:27
    //val =x223 // 9-26 14:30:00
    val=0x23;
    RTC_WRITE(TPS65910_RTC_COMP_LSB,1,val);
    val=0x02;
    RTC_WRITE(TPS65910_RTC_COMP_MSB,1,val);
    //printk("=========no compensation======TPS65910_DEVCTRL=%x\n",val);

    RTC_READ(TPS65910_RTC_CTRL,1,val);
    // val  |= BIT(0); /* startup rtc */
    val  |= BIT(2);/*Auto compensation enabled*/
    //val  &=~ BIT(2);/*Auto compensation disable*/
    //val  |= BIT(5);/*set the 32-kHz counter with COMP_REG value*/
    //val  &= ~ BIT(5);/*disable the 32-kHz counter with COMP_REG value*/
    RTC_WRITE(TPS65910_RTC_CTRL,1,val);

    RTC_READ(TPS65910_RTC_CTRL,1,val);
    val  |= BIT(0);/*startup rtc*/    
    RTC_WRITE(TPS65910_RTC_CTRL,1,val);
    
    //RTC_READ(TPS65910_RTC_CTRL,1,val);
    //printk("===============TPS65910_RTC_CTRL=%x\n",val);
    //RTC_READ(TPS65910_RTC_RES_PROG,1,val);
    //printk("===============TPS65910_RTC_RES_PROG=%x\n",val);
    //RTC_READ(TPS65910_RTC_RESET_STATUS,1,val);
    //printk("===============TPS65910_RTC_RESET_STATUS=%x\n",val);

     //the following is the code of charging backup bettery
    //RTC_READ(TPS65910_BBCH,1,val);
    //val  |= BIT(0);/*select backup bettary for supplying  rtc power*/
    //RTC_WRITE(TPS65910_BBCH,1,val);

    /*NOTE: RTC backup bettery must be bigger than 3V,and 2.9V is bad*/
    
    spin_unlock(&rtc_dev.lock);
    
    return 0;
}

int rtc_close(struct inode *inode, struct file *flip)
{
    
    return 0;
}

/*binary-coded decimal*/
#define BCD(x) ((x>>4)*10 + (x & 0xf))
#define DCB(x) ((x/10)*16 + (x%10))

static ssize_t rtc_read(struct file *file, const char __user *buf,
						size_t count, loff_t *ppos)
{
    char reg,val;
    int tm[TIME_LEN]={0};
    int len=0,num=0;
    int i=0;
    
    len = count > TIME_LEN*sizeof(int) ? TIME_LEN*sizeof(int) : count;
    num=len/sizeof(int);
    
    spin_lock(&rtc_dev.lock);
    for(i=0;i<num;i++)
    {
        //DBG_PRT("reg=%x\n",i);
        RTC_READ(i,1,val);
        //DBG_PRT("reg=%x,val=%x\n",i,val);
        tm[i] = BCD(val);
        if(i==YEAR_POS) tm[i] += 2000;/*year=00-99*/
    }
    spin_unlock(&rtc_dev.lock);
    copy_to_user(buf,tm,len);
    
    return len;
}

int valid_time(int i,int data)
{
    int ret=0;
    
    if(i<0 || i>=TIME_LEN) return 0;
    
    if(data>=time_range[i][0] && data<=time_range[i][1])
       ret=1;
    else
        ret=0;
    
    return ret;
}

int leap_year(int y)
{
    if(y%400 == 0)
        return 1;
    if((y%4==0) && (y%100 != 0))
        return 1;
    
    return 0;
}

int valid_day(int y,int m,int d)
{
    int ret=0;
    
    if(m<1 || m> 12) return 0;
    
    if(leap_year(y))
    {
        if((d>=days_range_leap[m-1][0]) && (d<=days_range_leap[m-1][1]))
            return 1;
        else 
            return 0;
    }
    else
    {
        if((d>=days_range_common[m-1][0]) && (d<=days_range_common[m-1][1]))
            return 1;
        else 
            return 0;        
    }
    
}


int time_check(int tm[TIME_LEN],int len)
{
    int i;
    for(i=0;i<len;i++)
    {
            if(tm[i] == 0xff) continue;/*skip setting */
            
            if((valid_time(i,tm[i])==0) )
            {
                //DBG_PRT("invalid time %s\n",time_print[i]);
                return 0;
            }
    }

    if(len<=3) return 1; /*no need to check days*/

    /*days , months, years  avoid skipping setting error*/
    if((tm[3]  == 0xff) && (tm[4]==0xff) && (tm[5]==0xff))
        return 1;
    else
        if((tm[3] == 0xff) || (tm[4]==0xff) || (tm[5]==0xff))
        {
            //DBG_PRT("days,month,year must set 255  together\n");
            return 0;
        }
        
    /*check days by month and year*/
    if((valid_day(tm[5],tm[4],tm[3])==0))
    {
                //DBG_PRT("days set error\n");
                return 0;
    }
    
    return 1;
}

static ssize_t rtc_write(struct file *file, const char __user *buf,
						size_t count, loff_t *ppos)
{
     char reg,val;
     int len=0,num=0;
     int i=0;
     int tm[TIME_LEN]={0};

     //DBG_PRT("\ncount=%d\n",count);

     len = count > TIME_LEN*sizeof(int) ? TIME_LEN*sizeof(int) : count;
     num=len/sizeof(int);
     copy_from_user(tm,buf,len);

    DBG_PRT("rtc_write : %.4d-%.2d-%.2d %.2d:%.2d:%.2d weeks:%d\n",
          tm[5],tm[4],tm[3],tm[2],tm[1],tm[0],tm[6]);
     
     spin_lock(&rtc_dev.lock);
     if(time_check(tm,num)==0)
    {
                    printk("time check time error\n");
                    return 0;
    }
     
     for(i=0;i<num;i++)
     {  
        if(tm[i] == 0xff)  continue;/*skip setting */
        if(i==YEAR_POS) tm[i] -= 2000;/*year=00-99*/
         val=DCB(tm[i]);
         //DBG_PRT("reg=%x,val=%x\n",i,val);
         RTC_WRITE(i,1,val);
     }
      spin_unlock(&rtc_dev.lock);
     
     return len;
}

static long rtc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}


static struct file_operations rtc_ops={
    .owner		= THIS_MODULE,
    .open = rtc_open,
    .read=rtc_read,
    .write=rtc_write,
    .unlocked_ioctl=rtc_ioctl,
    .release = rtc_close,
};

static int __devinit tps65910_rtc_probe(struct platform_device *pdev)
{
    struct tps65910 *tps65910=dev_get_platdata(&pdev->dev);

    if(!tps65910)
    {
        printk("tps65910 rtc  failed\n");
        return -1;
    }
    printk("probe tps65910 rtc\n");
    rtc_dev.tps65910=tps65910;
    rtc_dev.major = register_chrdev(0, rtc_dev.name, &rtc_ops); 
    rtc_dev.rtc_class = class_create(THIS_MODULE,rtc_dev.name);
    rtc_dev.dev= device_create(rtc_dev.rtc_class, NULL, MKDEV(rtc_dev.major, 0), NULL,rtc_dev.name); 

    spin_lock_init(&rtc_dev.lock);

    return 0;
}

static struct platform_driver tps65910_rtc_driver = {
	.probe		= tps65910_rtc_probe,
	.driver		= {
		.name	= "tps65910-rtc",
		.owner	= THIS_MODULE,
	},
};

static int __init tps65910_rtc_init(void)
{

	return platform_driver_register(&tps65910_rtc_driver);
}

static void __exit tps65910_rtc_exit(void)
{
	platform_driver_unregister(&tps65910_rtc_driver);
}

module_init(tps65910_rtc_init);
module_exit(tps65910_rtc_exit);


MODULE_LICENSE("GPL");


