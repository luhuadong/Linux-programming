/*
 * Copyright (c) 2003-2021, Guangzhou Bocon Ltd.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-12     liyaolong    first version
 * 2021-03-15     luhuadong    add support for imx8
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/cdev.h>

#include "spi-protocol.h"

//#define DEBUG_PRT printk
#define DEBUG_PRT(arg...)

#define HIGH_LEVEL              1
#define LOW_LEVEL               0

#define DODEV_MAJOR             241
#define DIDEV_MAJOR             242
#define AIDEV_MAJOR             243

#define DEV_REBOOT_MAJOR        237

#define AI_CONF_VOLT_CURR       _IOW('k',1,int)
#define DO_CONF_PLUSE_PERIOD    _IOW('k',2,int)
#define DI_GET_PLUSE_COUNT      _IOW('k',3,int)
#define DI_CLR_PLUSE_COUNT      _IOW('k',4,int)
#define AI_SET_VOLT_CURR_CAL    _IOW('k',5,int)
#define AI_GET_VOLT_CURR_CAL    _IOW('k',6,int)
#define AI_SET_ANALOG_MODE      _IOW('T',7,int)
#define AI_GET_ANALOG_VALUE     _IOW('T',8,int)

struct  dodev {
    dev_t devt;
    struct device *parent;
    struct device *dev;
    int users;
    int chan;
    struct semaphore sem;
    struct protocol_data *pro;
};

struct  didev {
    dev_t devt;
    struct device *parent;
    struct device *dev;
    int users;
    int chan;
    struct semaphore sem;
    struct protocol_data *pro;
};

struct  aidev {
    dev_t devt;
    struct device *parent;
    struct device *dev;
    int users;
    int chan;
    struct semaphore sem;
    struct protocol_data *pro;
};

struct  dev_reboot {
    dev_t devt;
    struct device *parent;
    struct device *dev;
    int users;
    struct semaphore sem;
    struct protocol_data *pro;
};

struct ai_type {
    char ai;
    char type;
};

struct ai_calibra {
    char dev;
    struct ai_calibrate ai_cal;
};

struct ai_analog_value {
    char dev;
    struct analog_value analog_value;
};

struct ai_type_list {
    char ai_count;
    struct ai_type ai_type[MCU_MAX_AI_NUM];
};

static struct dodev * dodev;
static struct class * dodev_class;

static struct didev * didev;
static struct class * didev_class;

static struct aidev * aidev;
static struct class * aidev_class;

static struct dev_reboot * dev_reboot;
static struct class * class_dev_reboot;

static ssize_t
dodev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct  dodev *pdo=NULL;
    int val;

    pdo = filp->private_data;
    if(!pdo) {
        printk("PDO NULL porinter\n");
        return -1;
    }
    down(&pdo->sem);
    val = get_do(pdo->chan+1, pdo->pro);
    up(&pdo->sem);
    
    //printk("chan=%d,val=%x\n",pdo->chan,val);

    if(copy_to_user(buf,&val,sizeof(int)))
        return -1;

    return sizeof(val);
}

static ssize_t
dodev_write(struct file *filp,const char __user *buf, size_t count, loff_t *f_pos)
{
    struct  dodev *pdo = NULL;
    int val;
    int ret = 0;
    
    ret = copy_from_user(&val,buf,sizeof(int));
    if(ret != 0) 
        printk("copy_from_user fail %s \n",__func__);
    
    pdo=filp->private_data;
    if(!pdo) {
        printk("PDO NULL porinter\n");
        return -1;
    }
    //printk("chan=%d,val=%x\n",pdo->chan,val);
    down(&pdo->sem);
    ret=set_do_voltage(pdo->chan+1, val,pdo->pro);
    up(&pdo->sem);
    if(ret!=0) printk("set DO voltage error ret=%d\n",ret);

    return sizeof(val);
}

static int  dodev_open(struct inode *inode, struct file *filp)
{
    struct  dodev *pdo = NULL;
    int i = 0;

    for(i=0; i<_g_mcu_describes.do_num; i++) {
        if(dodev[i].devt == inode->i_rdev) {
            pdo = &dodev[i];
            break;
        }
    }

    if(pdo) {
        pdo->users++;
        filp->private_data = pdo;
    }
    return 0;
}

static long dodev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct dodev *pdo = NULL;
    int ret = 0;
    int residue = 0;
    
    pdo = filp->private_data;
    if(!pdo) {
        printk("PDO NULL pointer\n");
        return -1;
    }

    down(&pdo->sem);  
    switch(cmd) {
        case DO_CONF_PLUSE_PERIOD :
        {
            unsigned short period = 0;
            int dev = pdo->chan+1;
            residue = copy_from_user(&period, (unsigned short *)arg, sizeof(unsigned short));
            ret = set_do_pulse(dev, period, pdo->pro);
            if(ret != 0 || residue > 0) {
                ret = -1;
                goto DODEV_IOCTL_ERROR;
            }
        }
        break;
        default : ret = -1;
    }
    
    DODEV_IOCTL_ERROR:
    up(&pdo->sem);
    return ret;
}

static int mcu_do_probe(struct platform_device *pdev)
{
    struct protocol_data * pro = dev_get_platdata(&pdev->dev);
    struct  dodev *pdo;
    char do_name[10] = {0};
    int i,ret;

    dodev = pdo = kmalloc(sizeof(struct  dodev)*_g_mcu_describes.do_num, GFP_KERNEL);
    memset(pdo, 0, sizeof(struct dodev)*_g_mcu_describes.do_num);

    for(i=0; i<_g_mcu_describes.do_num; i++)
    {
        pdo[i].pro    = pro;
        sema_init(&pdo[i].sem, 1);
        pdo[i].chan   = i;
        pdo[i].parent = &pdev->dev;
        pdo[i].devt   = MKDEV(DODEV_MAJOR, i);
        sprintf(do_name, "dodev%d", i);
        pdo[i].dev    = device_create(dodev_class, pdo[i].parent, pdo[i].devt,&pdo[i], do_name);
        ret = IS_ERR(pdo[i].dev ) ? PTR_ERR(pdo[i].dev ) : 0;
        if(ret != 0) break;
    }
    return ret;
}

static int mcu_di_probe(struct platform_device *pdev)
{
    struct protocol_data * pro = dev_get_platdata(&pdev->dev);
    struct  didev *pdi;
    char di_name[10] = {0};
    int i,ret;

    //printk("============mcu_di_probe\n");

    didev = pdi = kmalloc(sizeof(struct  didev)*_g_mcu_describes.di_num,GFP_KERNEL);
    memset(pdi, 0, sizeof(struct  didev)*_g_mcu_describes.di_num);

    for(i=0; i<_g_mcu_describes.di_num; i++)
    {
        pdi[i].pro=pro;
        sema_init(&pdi[i].sem,1);
        pdi[i].chan=i;
        pdi[i].parent = &pdev->dev;
        pdi[i].devt = MKDEV(DIDEV_MAJOR, i);
        sprintf(di_name,"didev%d",i);
        pdi[i].dev = device_create(didev_class, pdi[i].parent, pdi[i].devt,&pdi[i], di_name);
        ret = IS_ERR(pdi[i].dev ) ? PTR_ERR(pdi[i].dev ) : 0;   
        if(ret!=0) break;
    }
    return ret;
}

static int  didev_open(struct inode *inode, struct file *filp)
{
    struct  didev *pdi = NULL;
    int i = 0;

    for(i=0;i<_g_mcu_describes.di_num;i++) {
        if(didev[i].devt == inode->i_rdev) {
            pdi = &didev[i];
            break;
        }
    }

    if(pdi) {
        pdi->users++;
        filp->private_data =  pdi;
    }
    
    return 0;
}

static ssize_t
didev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct  didev *pdi = NULL;
    int val;

    pdi = filp->private_data;
    if(!pdi) {
        printk("PDI NULL porinter\n");
        return -1;
    }
    down(&pdi->sem);  
    val = get_di(pdi->chan+1,pdi->pro);
    up(&pdi->sem);

    if(copy_to_user(buf, &val, sizeof(int)))
        return -1;

    return sizeof(val);
}

static long didev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct  didev *pdi = NULL;
    int ret = 0;
    int residue = 0;
    
    pdi = filp->private_data;
    if(!pdi) {
        printk("PDI NULL pointer\n");
        return -1;
    }

    down(&pdi->sem);  
    switch(cmd)
    {
        case DI_GET_PLUSE_COUNT :
        {
            unsigned long counter = 0;
            int dev = pdi->chan+1;
            ret = get_di_counters(dev,&counter,pdi->pro);
            if(ret != 0) {
                ret = -1;
                goto DIDEV_IOCTL_ERROR;
            }

            residue = copy_to_user((unsigned long *)arg, &counter,sizeof(unsigned long));
            if(residue) 
                printk("%s %d copy_to_user FAIL \n", __func__, __LINE__);
        }
        break;
        case DI_CLR_PLUSE_COUNT:
        {
            int dev = pdi->chan+1;
            ret = clear_di_counters(dev,pdi->pro);
            if(ret != 0)
            {
                ret = -1;
                goto DIDEV_IOCTL_ERROR;
            }
        }
        break;
        default : ret=-1;    
    }
    
    DIDEV_IOCTL_ERROR:
    up(&pdi->sem);

    return ret;
}


static int mcu_ai_probe(struct platform_device *pdev)
{
    struct protocol_data *pro = dev_get_platdata(&pdev->dev);
    struct  aidev *pai;
    char ai_name[10]={0};
    int i,ret;

    //printk("============mcu_ai_probe\n");
    
    aidev=pai=kmalloc(sizeof(struct  aidev)*_g_mcu_describes.ai_num,GFP_KERNEL);
    memset(pai,0,sizeof(struct  aidev)*_g_mcu_describes.ai_num);
    
    for(i=0;i<_g_mcu_describes.ai_num;i++)
    {
        pai[i].pro=pro;
        sema_init(&pai[i].sem,1);
        pai[i].chan=i;
        pai[i].parent = &pdev->dev;
        pai[i].devt = MKDEV(AIDEV_MAJOR, i);
        sprintf(ai_name,"aidev%d",i);
        pai[i].dev = device_create(aidev_class, pai[i].parent, pai[i].devt,&pai[i], ai_name);
        ret = IS_ERR(pai[i].dev ) ? PTR_ERR(pai[i].dev ) : 0;   
        if(ret != 0) break;
    }
    return ret;
}

static int  aidev_open(struct inode *inode, struct file *filp)
{
    struct aidev *pai = NULL;
    int i = 0;

    for(i=0; i<_g_mcu_describes.ai_num; i++) {
        if(aidev[i].devt == inode->i_rdev) {
            pai = &aidev[i];
            break;
        }
    }

    if(pai) {
        pai->users++;
        filp->private_data = pai;
    }
    return 0;
}

static ssize_t
aidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct aidev *pai = NULL;
    float  val;

    pai = filp->private_data;
    if(!pai) 
    {
        printk("PDI NULL porinter\n");
        return -1;
    }
    down(&pai->sem);

    //kernel_fpu_begin();
    val = get_ai(pai->chan+1, pai->pro);
    //kernel_fpu_end();

    up(&pai->sem);
    
    if(copy_to_user(buf, &val, sizeof(float)))
        return -1;

    return sizeof(val);
}

static long aidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct  aidev *pai = NULL;
    int ret = 0;
    int residue = 0;

    pai = filp->private_data;
    if(!pai) {
        printk("PDI NULL porinter\n");
        return -1;
    }
    
    down(&pai->sem);  
    switch(cmd)
    {
        case AI_CONF_VOLT_CURR :
        {
            /*
                struct ai_type ai_conf; 
           
                memset(&ai_conf,0,sizeof(struct ai_type));
                copy_from_user(&ai_conf,(char *)arg,sizeof(struct ai_type));

                ret=set_ai_type(ai_conf.ai + 1,ai_conf.type,pai->pro);
                if(ret!=0)
                {
                    ret=-1;
                    goto AIDEV_IOCTL_ERROR;
                }*/
            ret = 0;
        }
        break;

        case AI_SET_VOLT_CURR_CAL :
        {
            struct ai_calibra ai_calValue;

            memset(&ai_calValue,0,sizeof(struct ai_calibra));
            residue = copy_from_user(&ai_calValue,(char *)arg,sizeof(struct ai_calibra));
            ret=sync_ai_calibrates(ai_calValue.dev + 1,&ai_calValue.ai_cal,pai->pro);
            if(ret!=0)
            {
                ret=-1;
                goto AIDEV_IOCTL_ERROR;
            }
        }
        break;

        case AI_GET_VOLT_CURR_CAL :
        {
            struct ai_calibra ai_calValue;

            memset(&ai_calValue,0,sizeof(struct ai_calibra));
            residue = copy_from_user(&ai_calValue,(char *)arg,sizeof(struct ai_calibra));
            ret=get_ai_calibrates(ai_calValue.dev + 1,&ai_calValue.ai_cal,pai->pro);
            if(ret!=0)
            {
                ret=-1;
                goto AIDEV_IOCTL_ERROR;
            }
            residue = copy_to_user((char *)arg,&ai_calValue,sizeof(struct ai_calibra));
        }
        break;
            // new add
        case AI_SET_ANALOG_MODE :
        {
            int analog_mode = 0;

                //printk("AI_SET_ANALOG_MODE\n");
            residue = copy_from_user(&analog_mode,(char *)arg,sizeof(int));
            ret=set_mcu_analog_mode(analog_mode,pai->pro);
            if(ret!=0)
            {
                ret=-1;
                goto AIDEV_IOCTL_ERROR;
            }
        }
        break;

        case AI_GET_ANALOG_VALUE :
        {
            struct ai_analog_value analog_value;

            //printk("AI_GET_ANALOG_VALUE\n");
            memset(&analog_value,0,sizeof(struct ai_analog_value));
            residue = copy_from_user(&analog_value,(char *)arg,sizeof(struct ai_analog_value));
            ret=get_mcu_analog_value(analog_value.dev + 1,&analog_value.analog_value,pai->pro);
            if(ret!=0)
            {
                ret=-1;
                goto AIDEV_IOCTL_ERROR;
            }
            residue = copy_to_user((char *)arg,&analog_value,sizeof(struct ai_analog_value));
        }
        break;
            //

        default : ret=-1;    
    }
    
    if(residue)
        printk("%s %d kernel user copy maybe fail \n", __func__, __LINE__);

AIDEV_IOCTL_ERROR:
    up(&pai->sem);
    return ret;
}

static int  dev_reboot_open(struct inode *inode, struct file *filp)
{
    struct  dev_reboot *pdev_reboot = dev_reboot;

    pdev_reboot->users++;
    filp->private_data = pdev_reboot;
    
    return 0;
}

static ssize_t
dev_reboot_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct  dev_reboot *pdev_reboot = NULL;

    pdev_reboot = filp->private_data;
    if(!pdev_reboot) {
        printk("pdev_reboot NULL porinter\n");
        return -1;
    }

    reset_reboot_count(pdev_reboot->pro);

    return sizeof(int);
}

static int dev_reboot_probe(struct platform_device *pdev)
{
    struct protocol_data * pro = dev_get_platdata(&pdev->dev);
    struct  dev_reboot *pdev_reboot;
    char dev_reboot_name[100] = {0};
    int ret;

    //printk("============dev_reboot_probe\n");
    
    dev_reboot = pdev_reboot = kmalloc(sizeof(struct  dev_reboot),GFP_KERNEL);
    memset(pdev_reboot, 0, sizeof(struct  dev_reboot));
    
    pdev_reboot->pro = pro;
    sema_init(&pdev_reboot->sem, 1);
    pdev_reboot->parent = &pdev->dev;
    pdev_reboot->devt = MKDEV(DEV_REBOOT_MAJOR, 0);
    strcpy(dev_reboot_name,"dev_reboot");
    pdev_reboot->dev = device_create(class_dev_reboot, pdev_reboot->parent, pdev_reboot->devt,pdev_reboot, dev_reboot_name);
    ret = IS_ERR(pdev_reboot->dev ) ? PTR_ERR(pdev_reboot->dev ) : 0;
    
    return ret;
}

static const struct file_operations dodev_fops = {
    .owner =    THIS_MODULE,
    .read  =    dodev_read,
    .write =    dodev_write,
    .open  =    dodev_open,
    .unlocked_ioctl = dodev_ioctl, 
};

static struct platform_driver mcu_do_driver = {
    .probe = mcu_do_probe,
    .driver = {
        .name = "mcu-do",
        .owner = THIS_MODULE,
    },
};

static const struct file_operations didev_fops = {
    .owner =    THIS_MODULE,
    .read  =    didev_read,
    .open  =    didev_open,
    .unlocked_ioctl = didev_ioctl,
};

static struct platform_driver mcu_di_driver = {
    .probe = mcu_di_probe,
    .driver = {
        .name = "mcu-di",
        .owner = THIS_MODULE,
    },
};

static const struct file_operations aidev_fops = {
    .owner =    THIS_MODULE,
    .read  =    aidev_read,
    .open  =    aidev_open,
    .unlocked_ioctl = aidev_ioctl,
};

static struct platform_driver mcu_ai_driver = {
    .probe = mcu_ai_probe,
    .driver = {
        .name = "mcu-ai",
        .owner = THIS_MODULE,
    },
};

static const struct file_operations dev_reboot_fops = {
    .owner =    THIS_MODULE,
    .write =    dev_reboot_write,
    .open  =    dev_reboot_open,
};

static struct platform_driver dev_reboot_driver = {
    .probe = dev_reboot_probe,
    .driver = {
        .name = "dev_reboot",
        .owner = THIS_MODULE,
    },
};

static int __init aidodi_init(void)
{
    int status;

    /* DO */
    status = register_chrdev(DODEV_MAJOR, "dodev", &dodev_fops);
    if (status < 0)
    {
        printk("register do device errror\n");
        return status;
    }
    dodev_class = class_create(THIS_MODULE, "dodev");
    if (IS_ERR(dodev_class)) {
        unregister_chrdev(DODEV_MAJOR," dodev");
        return PTR_ERR(dodev_class);
    }
    platform_driver_register(&mcu_do_driver);

    /* DI */
    status = register_chrdev(DIDEV_MAJOR, "didev", &didev_fops);
    if (status < 0)
    {
       printk("register di device errror\n");
       return status;
    }
    didev_class = class_create(THIS_MODULE, "didev");
    if (IS_ERR(didev_class)) {
        unregister_chrdev(DIDEV_MAJOR," didev");
        return PTR_ERR(didev_class);
    }
    platform_driver_register(&mcu_di_driver);

    /* AI */
    status = register_chrdev(AIDEV_MAJOR, "aidev", &aidev_fops);
    if (status < 0)
    {
        printk("register ai device errror\n");
        return status;
    }
    aidev_class = class_create(THIS_MODULE, "aidev");
    if (IS_ERR(aidev_class)) {
        unregister_chrdev(AIDEV_MAJOR," aidev");
        return PTR_ERR(aidev_class);
    }
    platform_driver_register(&mcu_ai_driver);

    /* Reboot */
    class_dev_reboot = class_create(THIS_MODULE, "dev_reboot");
    if (IS_ERR(class_dev_reboot)) {
        unregister_chrdev(DEV_REBOOT_MAJOR," dev_reboot");
        return PTR_ERR(class_dev_reboot);
    }
    platform_driver_register(&dev_reboot_driver);

    status = register_chrdev(DEV_REBOOT_MAJOR, "dev_reboot", &dev_reboot_fops);
    if (status < 0)
    {
        printk("register dev_reboot  device errror\n");
        return status;
    }

    return 0;
}

static void __exit aidodi_exit(void)
{
    platform_driver_unregister(&mcu_do_driver);
    class_destroy(dodev_class);
    unregister_chrdev(DODEV_MAJOR," dodev");

    platform_driver_unregister(&mcu_di_driver);
    class_destroy(didev_class);
    unregister_chrdev(DIDEV_MAJOR," didev");

    platform_driver_unregister(&mcu_ai_driver);
    class_destroy(aidev_class);
    unregister_chrdev(AIDEV_MAJOR," aidev");

    platform_driver_unregister(&dev_reboot_driver);
    class_destroy(class_dev_reboot);
    unregister_chrdev(DEV_REBOOT_MAJOR," dev_reboot");
}

module_init(aidodi_init);
module_exit(aidodi_exit);

MODULE_LICENSE("GPL");