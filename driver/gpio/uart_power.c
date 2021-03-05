/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   uart_power.c
*   Author：     luhuadong
*   Create Date：2021年03月03日
*   Description：
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

#define UART_PWR_EN      129       /*UART_PWR_EN  GPIO5_01*/

int major;
struct class  *uart_power_class;
struct device *uart_power_class_devs;

static int uart_power_chrdev_open(struct inode *inode, struct file *filp)
{
    printk("\n open form driver \n");
    return 0;
}

static ssize_t uart_power_chrdev_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    unsigned char write_data; //用于保存接收到的数据

    int error = copy_from_user(&write_data, buf, cnt);
    if(error < 0) {
            return -1;
    }

    printk("\n write data %d\n", write_data);

    return 0;
}

static struct file_operations uart_power_drv_fops =
{
    .owner = THIS_MODULE,
    .open = uart_power_chrdev_open,
    .write = uart_power_chrdev_write,
};

static long uart_power_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{   
    printk("gpio_ioctl\n");
    switch(cmd) {

        case 1:
            if(arg == 0)
            {
                gpio_set_value(UART_PWR_EN, 0);
                printk("uart power off\n");
                break;
            }
            else {
                gpio_set_value(UART_PWR_EN, 1);
                printk("uart power on\n");
                break;
            }
        default:
                return -EINVAL;
    }
    return 0;
}

static  int uart_power_probe(struct platform_device *pdev)
{
    int ret;
    ret = gpio_request(UART_PWR_EN, "UART_PWR_EN");//第一个参数，为要申请的引脚，第二个为你要定义的名字
    
    if (ret) 
    {
        printk("[pual] gpio_request error %s - %d -- \n",__func__,__LINE__);
        return ret;
    }

    gpio_direction_output(UART_PWR_EN, 1);
    gpio_set_value(UART_PWR_EN, 1);
    major = register_chrdev(0, "uart-power", &uart_power_drv_fops);
    
    //创建设备信息，执行后会出现 /sys/class/uart-power
    uart_power_class = class_create(THIS_MODULE, "uart-power");

    //创建设备节点，就是根据上面的设备信息来的
    uart_power_class_devs = device_create(uart_power_class, NULL, MKDEV(major, 0), NULL, "uart-power"); /* /dev/uart-power */
    
    return 0;   
}

static const struct of_device_id uart_power_ids[] = {
    
    { .compatible = "gpio-uart-power", },
    { },
};
//MODULE_DEVICE_TABLE(of, uart_power_ids);

static struct platform_driver uart_power_driver={
    
    .driver = {
        .name  = "uart_power",
        .owner = THIS_MODULE,
        .of_match_table = uart_power_ids,
    },
    .probe  = uart_power_probe,
    //.remove = uart_power_remove,
};
//module_platform_driver(uart_power_driver); /* call module_init() */

static int __init uart_power_init(void)
{
    /*2. 注册平台驱动*/
    platform_driver_register(&uart_power_driver);
    return 0;
}

static void __exit uart_power_exit(void)
{
    /*3. 注销平台驱动*/
    platform_driver_unregister(&uart_power_driver);
}

module_init(uart_power_init);
module_exit(uart_power_exit);

MODULE_AUTHOR("Rudy Lo <luhuadong@163.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Bocon UART Power Driver");
MODULE_ALIAS("a uart power module");