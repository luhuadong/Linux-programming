/*====================================================================
*   Copyright (C) 2021 Guangzhou Dreamgrow Ltd. All rights reserved.
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
====================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>

#define DEFAULT_RTC_DEVICE   "rtc0"
#define TIME_LEN             7

#define GLOBALMEM_SIZE       0x1000  /* 4KB */
#define MEM_CLEAR            0x1
#define GLOBALMEM_MAJOR      230

static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;

struct class  *bocon_rtc_class;
struct device *bocon_rtc_class_devs;

static int globalmem_open(struct inode *inode, struct file *filp)
{
    filp->private_data = globalmem_devp;
    return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;

    switch (cmd) {
    case MEM_CLEAR:
        memset(dev->mem, 0, GLOBALMEM_SIZE);
        printk(KERN_INFO "globalmem is set to zero\n");
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
#if 0
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;

    if (p > GLOBALMEM_SIZE)
        return 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    if (copy_to_user(buf, dev->mem + p, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);
    }

    return ret;
#else
    int err = -ENODEV;
    struct rtc_time tm;
    int data[TIME_LEN]={0};
    unsigned int count = size;

    struct rtc_device *rtc = rtc_class_open(DEFAULT_RTC_DEVICE);

    if (!rtc) {
        pr_info("unable to open rtc device (%s)\n", DEFAULT_RTC_DEVICE);
        return err;
    }

    err = rtc_read_time(rtc, &tm);
    if (err) {
        dev_err(rtc->dev.parent, "hctosys: unable to read the hardware clock\n");
        rtc_class_close(rtc);
        return err;
    }

    data[0] = tm.tm_sec;
    data[1] = tm.tm_min;
    data[2] = tm.tm_hour;
    data[3] = tm.tm_mday;
    data[4] = tm.tm_mon + 1;
    data[5] = tm.tm_year + 1900;
    data[6] = tm.tm_wday;

    printk(KERN_INFO "Current RTC date/time is %d-%d-%d, %02d:%02d:%02d, %d\n",
            data[5], data[4], data[3], data[2], data[1], data[0], data[6]);

    if (size > sizeof(int) * TIME_LEN)
        count = sizeof(int) * TIME_LEN;

    if (copy_to_user(buf, data, count)) {
        err = -EFAULT;
    } else {
        err = count;
        printk(KERN_INFO "read %u bytes(s) from %s\n", count, DEFAULT_RTC_DEVICE);
    }

    return err;
#endif
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
#if 0
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;

    if (p > GLOBALMEM_SIZE)
        return 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    if (copy_from_user(dev->mem + p, buf, count)) {
        return -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "written %u bytes(s) from %lu\n", count, p);
    }

    return ret;
#else
    int err = -ENODEV;
    struct rtc_time tm;
    int data[TIME_LEN]={0};
    unsigned int count = size;

    struct rtc_device *rtc = rtc_class_open(DEFAULT_RTC_DEVICE);

    if (!rtc) {
        pr_info("unable to open rtc device (%s)\n", DEFAULT_RTC_DEVICE);
        return err;
    }

    if (!rtc->ops || !rtc->ops->set_time) {
        rtc_class_close(rtc);
        return err;
    }

    if (size > sizeof(int) * TIME_LEN)
        count = sizeof(int) * TIME_LEN;

    if (copy_from_user(data, buf, count)) {
        return -EFAULT;
    } else {
        //*ppos += count;
        err = count;
        printk(KERN_INFO "written %u bytes(s)\n", count);
    }

    tm.tm_sec  = data[0];
    tm.tm_min  = data[1];
    tm.tm_hour = data[2];
    tm.tm_mday = data[3];
    tm.tm_mon  = data[4] - 1;
    tm.tm_year = data[5] - 1900;
    tm.tm_wday = data[6];

    err = rtc_set_time(rtc, &tm);
    rtc_class_close(rtc);
    return err;
#endif
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret = 0;
    switch (orig) {
    case 0:
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        if ((unsigned int)offset > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset;
        ret = filp->f_pos;
        break;
    case 1:
        if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        if ((filp->f_pos + offset) < 0) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos += offset;
        ret = filp->f_pos;
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static const struct file_operations globalmem_fops = {
    .owner = THIS_MODULE,
    .llseek = globalmem_llseek,
    .read = globalmem_read,
    .write = globalmem_write,
    .unlocked_ioctl = globalmem_ioctl,
    .open = globalmem_open,
    .release = globalmem_release,
};

#if 0
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err, devno = MKDEV(globalmem_major, index);

    cdev_init(&dev->cdev, &globalmem_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding globalmem %d", err, index);
    }
}
#else
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int major = register_chrdev(0, "bocon-rtc", &globalmem_fops);
    
    //创建设备信息，执行后会出现 /sys/class/bocon-rtc
    bocon_rtc_class = class_create(THIS_MODULE, "bocon-rtc");

    //创建设备节点，就是根据上面的设备信息来的
    bocon_rtc_class_devs = device_create(bocon_rtc_class, NULL, MKDEV(major, 0), NULL, "bocon-rtc"); /* /dev/bocon-rtc */
}
#endif

static int __init globalmem_init(void)
{
    int ret;
    dev_t devno = MKDEV(globalmem_major, 0);

    if (globalmem_major) {
        ret = register_chrdev_region(devno, 1, "bocon-rtc");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, "bocon-rtc");
        globalmem_major = MAJOR(devno);
    }
    if (ret < 0)
        return ret;

    globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }

    globalmem_setup_cdev(globalmem_devp, 0);

    printk("bocon-rtc init\n");

    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return ret;
}

static void __exit globalmem_exit(void)
{
    cdev_del(&globalmem_devp->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
    printk("bocon-rtc exit\n");
}

module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_AUTHOR("luhuadong");
MODULE_LICENSE("GPL");
