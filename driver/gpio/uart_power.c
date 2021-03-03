/*================================================================
*   Copyright (C) 2021 Guangzhou Bocon Ltd. All rights reserved.
*   
*   Filename：   uart_power.c
*   Author：     luhuadong
*   Create Date：2018年05月08日
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

// #define USER_LED      374       /*GPIO LED  GPIO4_22*/
#define USER_LED       321       /*UART_PWR_EN  GPIO5_01*/

int major;
struct class *myled_class;
struct device *myled_class_devs;


static int led_chr_dev_open(struct inode *inode, struct file *filp)
{
    printk("\n open form driver \n");
    return 0;
}

static ssize_t led_chr_dev_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    unsigned char write_data; //用于保存接收到的数据

    int error = copy_from_user(&write_data, buf, cnt);
    if(error < 0) {
            return -1;
    }

    printk("\n write data %d\n", write_data);

    return 0;
}

static struct file_operations myled_drv_fops =
{
    .owner = THIS_MODULE,
    .open = led_chr_dev_open,
    .write = led_chr_dev_write,
};

static long myled_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{   
    printk("gpio_ioctl\n");
    switch(cmd) {

        case 1:
            if(arg == 0)
            {
                gpio_set_value(USER_LED, 0);
                printk("led is off\n");
                break;
            }
            else {
                gpio_set_value(USER_LED, 1);
                printk("led is on\n");
                break;
            }
        default:
                return -EINVAL;
    }
    return 0;
}

static  int myled_probe(struct platform_device *pdev)
{
    int ret;
    ret = gpio_request(USER_LED, "LED");//第一个参数，为要申请的引脚，第二个为你要定义的名字
    if (ret) 
    {
        printk("[pual] gpio_request error %s - %d -- \n",__func__,__LINE__);
                return ret;
       }
    gpio_direction_output(USER_LED, 1);
    gpio_set_value(USER_LED, 1);
    major = register_chrdev(0,"myled",&myled_drv_fops);
    
      //创建设备信息，执行后会出现 /sys/class/myled
      myled_class = class_create(THIS_MODULE, "myled");

    //创建设备节点，就是根据上面的设备信息来的
      myled_class_devs = device_create(myled_class, NULL, MKDEV(major, 0), NULL, "myled"); /* /dev/myled */
    
    return 0;   
}

static const struct of_device_id myled_ids[] = {
    
    { .compatible = "gpio-uart-power", },
    { },
};

static struct platform_driver myled_driver={
    .probe  = myled_probe,
    //.remove = myled_remove,
    .driver = {
        .name  = "myled",
        .of_match_table = myled_ids,
    }
};

static int __init myled_init(void)
{
    /*2. 注册平台驱动*/
    platform_driver_register(&myled_driver);
    return 0;
}

static void __exit myled_exit(void)
{

    /*3. 注销平台驱动*/
    platform_driver_unregister(&myled_driver);
    
}

module_init(myled_init);
module_exit(myled_exit);

//module_init(uart_power_platform_driver_init);
//module_exit(uart_power_platform_driver_exit);

MODULE_AUTHOR("Rudy Lo <luhuadong@163.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Bocon UART Power Driver");
MODULE_ALIAS("a uart power module");