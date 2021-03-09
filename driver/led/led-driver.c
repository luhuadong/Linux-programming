/*====================================================================
*   Copyright (C) 2021 Guangzhou Dreamgrow Ltd. All rights reserved.
*   
*   Filename：   led-driver.c
*   Author：     luhuadong
*   Create Date：2021年03月09日
*   Description：
*
====================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>

/* 设备结构体 */
struct light_dev {
    struct cdev cdev;     /* 字符设备 cdev 结构体 */
    unsigned char value;  /* LED 亮时为 1，熄灭时为 0，用户可读写此值 */
};

struct light_dev *light_devp;
int light_major = LIGHT_MAJOR;

int light_open(struct inode *inode, struct file *filp)
{
    struct light_dev *dev;

    /* 获得字符设备结构体指针 */
    dev = container_of(inode->i_cdev, struct light_dev, cdev);

    /* 让设备结构体作为设备的私有信息 */
    filp->private_data = dev;

    return 0;
}

int light_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t light_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct light_dev *dev = filp->private_data;  /* 获得设备结构体 */

    if (copy_to_user(buf, &(dev->value), 1))
        return -EFAULT;

    return 1;
}

ssize_t light_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct light_dev *dev = filp->private_data;  /* 获得设备结构体 */

    if (copy_from_user(&(dev->value), buf, 1))
        return -EFAULT;

    /* 根据写入的值点亮和熄灭 LED */
    if (dev->value == 1)
        light_on();
    else
        light_off();

    return 1;
}

int light_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct light_dev *dev = filp->private_data;  /* 获得设备结构体 */

    switch (cmd) {
    case LIGHT_ON:
        dev->value = 1;
        light_on();
        break;
    case LIGHT_OFF:
        dev->value = 0;
        light_off();
        break;
    default:
        return -ENOTTY;  /* 不能支持的命令 */
    }

    return 0;
}

struct file_operations light_fops = {
    .owner = THIS_MODULE,
    .read  = light_read,
    .write = light_write,
    .unlocked_ioctl = light_ioctl,
    .open  = light_open,
    .release = light_release,
};

/* 设置字符设备 cdev 结构体 */
static void light_setup_cdev(struct light_dev *dev, int index)
{
    int err, devno = MKDEV(light_major, index);

    cdev_init(&dev->cdev, &light_fops);

    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &light_fops;

    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

int light_init(void)
{
    int result;
    dev_t dev = MKDEV(light_major, 0);

    /* 申请字符设备号 */
    if (light_major)
        result = register_chrdev_region(dev, 1, "LED");
    else {
        result = alloc_chrdev_region(&dev, 0, 1, "LED");
        light_major = MAJOR(dev);
    }
    if (result < 0)
        return result;

    /* 分配设备结构体的内存 */
    light_devp = kmalloc(sizeof(struct light_dev), GFP_KERNEL);
    if (!light_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(light_devp, 0, sizeof(struct light_dev));
    light_setup_cdev(light_devp, 0);
    light_gpio_init();
    return 0;

fail_malloc:
    unregister_chrdev_region(dev, light_devp);
    return result;
}

void light_cleanup(void)
{
    cdev_del(&light_devp->cdev); /* 删除字符设备结构体 */
    kfree(light_devp);           /* 释放在 light_init 中分配的内存 */
    unregister_chrdev_region(MKDEV(light_major, 0), 1);  /* 字符设备注销 */
}

module_init(light_init);
module_exit(light_cleanup);

MODULE_AUTHOR("Rudy Lo <luhuadong@163.com>");
MODULE_LICENSE("Dual BSD/GPL");