/*================================================================
*   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：hello.c
*   创 建 者：luhuadong
*   创建日期：2018年05月08日
*   描    述：
*
================================================================*/


#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello World enter\n");

    printk(KERN_EMERG   "GetIot: KERN_EMERG\n");
    printk(KERN_ALERT   "GetIot: KERN_ALERT\n");
    printk(KERN_CRIT    "GetIot: KERN_CRIT\n");
    printk(KERN_ERR     "GetIot: KERN_ERR\n");
    printk(KERN_WARNING "GetIot: KERN_WARNING\n");
    printk(KERN_NOTICE  "GetIot: KERN_NOTICE\n");
    printk(KERN_INFO    "GetIot: KERN_INFO\n");
    printk(KERN_DEBUG   "GetIot: KERN_DEBUG\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Hello World exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Rudy Lo <luhuadong@163.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple Hello World module");
MODULE_ALIAS("a simplest module");
