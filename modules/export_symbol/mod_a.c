#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

int func1(void)
{
    printk("In Func: %s... \n", __func__);
    return 0;
}
EXPORT_SYMBOL(func1);

static int __init hello_init(void)
{
    printk("Module A, say hello!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk("Module A, exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL v2");