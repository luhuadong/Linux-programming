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
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
