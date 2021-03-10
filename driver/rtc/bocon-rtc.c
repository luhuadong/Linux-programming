/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   bocon-rtc.c
*   Author：     luhuadong
*   Create Date：2021年03月08日
*   Description：
*
*     cat /proc/devices
*     mknod /dev/bocon-rtc c 230 0
*     echo "hello world" > /dev/bocon-rtc
*     cat /dev/bocon-rtc
*
================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>

#define DYNAMIC_DEVICE_NUM   0
#define DEFAULT_DEVICENAME   "bocon-rtc"

#define DEFAULT_RTC_DEVICE   "rtc0"
#define RTC_DATA_SIZE        7

#define MEM_CLEAR            0x1
#define BOCON_RTC_MAJOR      230

static int bocon_rtc_major = BOCON_RTC_MAJOR;
module_param(bocon_rtc_major, int, S_IRUGO);

struct bocon_rtc_dev {
    struct cdev cdev;
    unsigned int data[RTC_DATA_SIZE];
};

struct bocon_rtc_dev *bocon_rtc_devp;

struct class  *bocon_rtc_class;
struct device *bocon_rtc_class_devs;

static int bocon_rtc_open(struct inode *inode, struct file *filp)
{
    filp->private_data = bocon_rtc_devp;
    printk(KERN_INFO "bocon-rtc is opened\n");
    return 0;
}

static int bocon_rtc_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "bocon-rtc is release\n");
    return 0;
}

static long bocon_rtc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct bocon_rtc_dev *dev = filp->private_data;

    switch (cmd) {
    case MEM_CLEAR:
        memset(dev->data, 0, sizeof(dev->data));
        printk(KERN_INFO "clear bocon rtc memory %lu bytes\n", sizeof(dev->data));
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static ssize_t bocon_rtc_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    /* not support ppos */

    int err = -ENODEV;
    struct rtc_time tm;
    unsigned int count = size;

    struct bocon_rtc_dev *dev = filp->private_data;

    struct rtc_device *rtc = rtc_class_open(DEFAULT_RTC_DEVICE);
    if (!rtc) {
        pr_info("unable to open rtc device (%s)\n", DEFAULT_RTC_DEVICE);
        return err;
    }

    err = rtc_read_time(rtc, &tm);
    if (err) {
        dev_err(rtc->dev.parent, "unable to read the hardware clock\n");
        rtc_class_close(rtc);
        return err;
    }

    dev->data[0] = tm.tm_sec;
    dev->data[1] = tm.tm_min;
    dev->data[2] = tm.tm_hour;
    dev->data[3] = tm.tm_mday;
    dev->data[4] = tm.tm_mon + 1;
    dev->data[5] = tm.tm_year + 1900;
    dev->data[6] = tm.tm_wday;

    /* check length */
    if (size > sizeof(dev->data))
        count = sizeof(dev->data);

    if (copy_to_user(buf, dev->data, count))
        return -EFAULT;

    printk(KERN_INFO "read %u bytes: %d-%d-%d, %02d:%02d:%02d, %d\n", count, 
            dev->data[5], dev->data[4], dev->data[3], dev->data[2], 
            dev->data[1], dev->data[0], dev->data[6]);

    return count;
}

static ssize_t bocon_rtc_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    /* not support ppos */

    int err = -ENODEV;
    struct rtc_time tm;
    unsigned int count = size;

    struct bocon_rtc_dev *dev = filp->private_data;

    struct rtc_device *rtc = rtc_class_open(DEFAULT_RTC_DEVICE);

    if (!rtc) {
        pr_info("unable to open rtc device (%s)\n", DEFAULT_RTC_DEVICE);
        return err;
    }

    if (!rtc->ops || !rtc->ops->set_time) {
        rtc_class_close(rtc);
        return err;
    }

    if (size > sizeof(dev->data))
        count = sizeof(dev->data);

    if (copy_from_user(dev->data, buf, count)) {
        return -EFAULT;
    }

    tm.tm_sec  = dev->data[0];
    tm.tm_min  = dev->data[1];
    tm.tm_hour = dev->data[2];
    tm.tm_mday = dev->data[3];
    tm.tm_mon  = dev->data[4] - 1;
    tm.tm_year = dev->data[5] - 1900;
    tm.tm_wday = dev->data[6];

    err = rtc_set_time(rtc, &tm);
    rtc_class_close(rtc);

    if (err < 0) {
        printk(KERN_INFO "write %u bytes failed\n", count);
        return err;
    }
    else {
        printk(KERN_INFO "write %u bytes\n", count);
        return count;
    }
}

static const struct file_operations bocon_rtc_fops = {
    .owner   = THIS_MODULE,
    .read    = bocon_rtc_read,
    .write   = bocon_rtc_write,
    .unlocked_ioctl = bocon_rtc_ioctl,
    .open    = bocon_rtc_open,
    .release = bocon_rtc_release,
};

static void bocon_rtc_setup_cdev(struct bocon_rtc_dev *dev, int index)
{
#if DYNAMIC_DEVICE_NUM
    int major = register_chrdev(0, DEFAULT_DEVICENAME, &bocon_rtc_fops);
#else
    int major = bocon_rtc_major;

    int err, devno = MKDEV(bocon_rtc_major, index);

    cdev_init(&dev->cdev, &bocon_rtc_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding bocon-rtc %d", err, index);
    }
#endif
    
    //创建设备信息，执行后会出现 /sys/class/bocon-rtc
    bocon_rtc_class = class_create(THIS_MODULE, DEFAULT_DEVICENAME);

    //创建设备节点 /dev/bocon-rtc，就是根据上面的设备信息来的
    bocon_rtc_class_devs = device_create(bocon_rtc_class, NULL, MKDEV(major, 0), NULL, DEFAULT_DEVICENAME);
}

static int __init bocon_rtc_init(void)
{
    int ret;
#if DYNAMIC_DEVICE_NUM

#else
    dev_t devno = MKDEV(bocon_rtc_major, 0);

    if (bocon_rtc_major) {
        ret = register_chrdev_region(devno, 1, DEFAULT_DEVICENAME);
        printk(KERN_INFO "register char device region, major = %d\n", bocon_rtc_major);
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, DEFAULT_DEVICENAME);
        bocon_rtc_major = MAJOR(devno);
        printk(KERN_INFO "register char device region, major = %d\n", bocon_rtc_major);
    }
    if (ret < 0)
        return ret;
#endif

    bocon_rtc_devp = kzalloc(sizeof(struct bocon_rtc_dev), GFP_KERNEL);
    if (!bocon_rtc_devp) {
        ret = -ENOMEM;
        unregister_chrdev_region(devno, 1);
        return ret;
    }

    bocon_rtc_setup_cdev(bocon_rtc_devp, 0);
    printk("bocon-rtc init\n");

    return 0;
}

static void __exit bocon_rtc_exit(void)
{
    if (bocon_rtc_class_devs) 
        device_destroy(bocon_rtc_class, MKDEV(bocon_rtc_major, 0));

    if (bocon_rtc_class) 
        class_destroy(bocon_rtc_class);

#if DYNAMIC_DEVICE_NUM
    unregister_chrdev(MKDEV(bocon_rtc_major, 0), DEFAULT_DEVICENAME);
#else
    cdev_del(&bocon_rtc_devp->cdev);
    kfree(bocon_rtc_devp);
    unregister_chrdev_region(MKDEV(bocon_rtc_major, 0), 1);
#endif
    printk("bocon-rtc exit\n");
}

module_init(bocon_rtc_init);
module_exit(bocon_rtc_exit);

MODULE_AUTHOR("luhuadong");
MODULE_LICENSE("GPL");
