/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   uart_power.c
*   Author：     luhuadong
*   Create Date：2021年03月03日
*   Description：Write file /dev/uart-power to switch on/off
*
================================================================*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#define UART_PWR_EN_PIN          129            /* GPIO5_01 */
#define DEFAULT_DEVICENAME       "uart-power"

int major;
struct class  *uart_power_class;
struct device *uart_power_class_devs;

static int uart_power_chrdev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "%s is opened\n", DEFAULT_DEVICENAME);
    return 0;
}

static int uart_power_chrdev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "%s is released\n", DEFAULT_DEVICENAME);
    return 0;
}

static ssize_t uart_power_chrdev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned int count = size, value = 0;

    value = gpio_get_value(UART_PWR_EN_PIN);

    if (size > sizeof(value))
        count = sizeof(value);

    if (copy_to_user(buf, &value, count))
        return -EFAULT;

    return count;
}

static ssize_t uart_power_chrdev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned int count = size, value = 0;

    if (size > sizeof(value))
        count = sizeof(value);

    if (copy_from_user(&value, buf, count)) {
        return -EFAULT;
    }

    if(value == 0)
    {
        gpio_set_value(UART_PWR_EN_PIN, 0);
        printk(KERN_INFO "uart power off\n");
    }
    else {
        gpio_set_value(UART_PWR_EN_PIN, 1);
        printk(KERN_INFO "uart power on\n");
    }

    return count;
}

static struct file_operations uart_power_drv_fops =
{
    .owner   = THIS_MODULE,
    .open    = uart_power_chrdev_open,
    .release = uart_power_chrdev_release,
    .read    = uart_power_chrdev_read,
    .write   = uart_power_chrdev_write,
};

static  int uart_power_probe(struct platform_device *pdev)
{
    int ret;
    ret = gpio_request(UART_PWR_EN_PIN, "UART_PWR_EN"); //第一个参数，为要申请的引脚，第二个为你要定义的名字
    if (ret) {
        dev_notice(&pdev->dev, "gpio_request error %s - %d -- \n", __func__,__LINE__);
        return ret;
    }

    gpio_direction_output(UART_PWR_EN_PIN, 1);
    gpio_set_value(UART_PWR_EN_PIN, 1);

    major = register_chrdev(0, DEFAULT_DEVICENAME, &uart_power_drv_fops);
    
    /* 创建设备信息，执行后会出现 /sys/class/uart-power */
    uart_power_class = class_create(THIS_MODULE, DEFAULT_DEVICENAME);

    /* 创建设备节点 /dev/uart-power，就是根据上面的设备信息来的 */
    uart_power_class_devs = device_create(uart_power_class, NULL, MKDEV(major, 0), NULL, DEFAULT_DEVICENAME);

    dev_info(&pdev->dev, "probe device ok!\n");
    
    return 0;
}

static int uart_power_remove(struct platform_device *pdev)
{
    if (uart_power_class_devs)
        device_destroy(uart_power_class, MKDEV(major, 0));

    if (uart_power_class)
        class_destroy(uart_power_class);

    unregister_chrdev(major, DEFAULT_DEVICENAME);
    gpio_free(UART_PWR_EN_PIN);

    dev_info(&pdev->dev, "remove device ok!\n");

    return 0;
}

static const struct of_device_id uart_power_ids[] = {
    
    { .compatible = "gpio-uart-power", },
    { },
};

static struct platform_driver uart_power_driver = {
    
    .driver = {
        .name  = "uart_power",
        .owner = THIS_MODULE,
        .of_match_table = uart_power_ids,
    },
    .probe  = uart_power_probe,
    .remove = uart_power_remove,
};

static int __init uart_power_init(void)
{
    platform_driver_register(&uart_power_driver);
    return 0;
}

static void __exit uart_power_exit(void)
{
    platform_driver_unregister(&uart_power_driver);
}

module_init(uart_power_init);
module_exit(uart_power_exit);

MODULE_AUTHOR("luhuadong");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bocon UART Power Driver");
