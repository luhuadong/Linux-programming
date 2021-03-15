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

#include "commands.h"
#include "spi-protocol.h"

//#define DEBUG_PRT printk
#define DEBUG_PRT(arg...)

#define K37A_DEV_MAJOR 240

#define CLR_K37A_MCU_LOG   _IOW('H',1,int)
#define GET_K37A_POWER_SUPPLY   _IOW('H',2,int)
#define GET_K37A_BATTERY_CHARGE   _IOW('H',3,int)
#define CUT_OFF_BATTERY_ALL_POWER_SUPPLY   _IOW('H',4,int)
#define START_MCU_UPDATE   			_IOW('H',5,int)
#define USE_NEW_MCU_FIRMWARE   		_IOW('H',6,int)
#define BATTERY_GET_VOLT_VALUE    _IOW('k',7,int)
#define BATTERY_SET_VOLT_CAL    _IOW('k',8,int)
#define BATTERY_GET_VOLT_CAL    _IOW('k',9,int)
#define MCU_MEMORY_TEST    		_IOW('k',10,int)
#define GET_MCU_MEMORY_TEST_RESULT    _IOW('k',11,int)

#define K37ADEV_NUM (ARRAY_SIZE(k37adev_str))
static char *k37adev_str[]={
    "sync",
     "ibutton",
     "door",
     "locker",
     "user",
     "log",
     "time",
     "temperature",
     "battery",
     "update",
     "mtest",	// memory test
	 "reboot", 
};

struct  k37adev {
    dev_t devt;
    struct device *parent;
    struct device *dev;
    int users;
    int chan;
    struct semaphore sem;
    struct protocol_data *pro;
};

struct mcu_log_t{
  char user;                //用户身份，0x03-上位机/远程，0x02-授权用户登陆K37A，0x01-未授权访问K37A，0x00-未登录
                            //用户身份，0x30-上位机/远程，0x20-授权访问站房，0x10-未授权访问站房，0x00-未登录
  char id[8];               //用户ID号
  char battery;             //电池电量，0-63，Bit6-充电1，未充电0，Bit7-有电/无电
  char deviceState;         //高4Bit-K37门状态，低4Bit-K37锁状态
  unsigned char tempK37;    //K37A设备内部温度，0-200 -> -100 ~ +100
  unsigned long time;       //数据产生的时间
};


static struct  k37adev * k37adev;
static struct class * k37adev_class;

extern int sync_user_infomation(int user_count,struct user_info *user_info,struct protocol_data * pro);
extern int get_ibutton_code(struct user_info *user_info,unsigned long timeout,struct protocol_data * pro);
//extern int set_k37a_door(char door,struct protocol_data * pro);
extern int sync_mcu_time(long seconds,struct protocol_data * pro);

extern char read_k37a_door(struct protocol_data * pro);
extern int set_k37a_locker(char locker,struct protocol_data * pro);
extern char read_k37a_locker(struct protocol_data * pro);
extern int get_current_user_info(struct user_info *user_info,struct protocol_data * pro);
extern unsigned char read_k37a_temperature(struct protocol_data * pro);
extern char read_reboot_reason(char * reason,struct protocol_data * pro);
extern int reboot_device(struct protocol_data * pro);

extern int get_mcu_log(char *order,char *log,int *log_len,unsigned long timeout,struct protocol_data * pro);
extern int clear_mcu_log(struct protocol_data * pro);
extern char read_battery_quantity(struct protocol_data * pro);
extern char read_power_supply(struct protocol_data * pro);
extern char read_battry_charge(struct protocol_data * pro);

extern int cut_off_battery_supply_power_all(struct protocol_data * pro);
extern int transfer_mcu_firmwares(struct firmware_packet *datas,int datas_len,
				struct transfer_result *result,int *result_len,struct protocol_data * pro);
extern int set_mcu_use_new_firmware(char *data,int * data_len,struct protocol_data * pro);
extern int start_mcu_firmware_update(struct firmware_update *datas,struct protocol_data * pro);
//new add
extern int get_battery_voltage(struct battery_voltage *bat_vol,struct protocol_data * pro);
extern int sync_battery_calibrates(struct battery_calibrate *bat_cal,struct protocol_data * pro);
extern int get_battery_calibrates(struct battery_calibrate *bat_cal,struct protocol_data * pro);
extern int start_mcu_memory_test(int memory_type,struct protocol_data * pro);
extern int get_mcu_memory_test_result(int memory_type,struct memory_result *mem_res,struct protocol_data * pro);

static ssize_t
k37adev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct  k37adev *k37a_dev=NULL;
    int val;
    int ret=0;
      
    k37a_dev=filp->private_data;
    if(!k37a_dev) 
    {
                printk("PDO NULL porinter\n");
                return -1;
    }
    down(&k37a_dev->sem);  
    switch(k37a_dev->chan)
    {
        case 0://users information
            break;//no used
            
        case 1://ibutton code
            {
                    struct user_info user_info;
                    ret=get_ibutton_code(&user_info,GET_IBUTTON_CODE_TIMEOUT,k37a_dev->pro);
                    if(ret!=0) 
                    {
                        ret=-1;
                        goto K37ADEV_READ_OUT;
                    }
                    else 
                    {
                            copy_to_user(buf,&user_info,sizeof(struct user_info));
                            ret= sizeof(struct user_info); 
                            goto K37ADEV_READ_OUT;
                    }
            }
            break;
        case 2://k37a door
            {
                    char door_status;
                    door_status=read_k37a_door(k37a_dev->pro);
                    copy_to_user(buf,&door_status,sizeof(door_status));
                    ret= sizeof(door_status);
                    goto K37ADEV_READ_OUT;
            }
            break;
        case 3://k37a locker
          {
                    char locker_status;
                    locker_status=read_k37a_locker(k37a_dev->pro);
                    copy_to_user(buf,&locker_status,sizeof(locker_status));
                    ret= sizeof(locker_status);
                    goto K37ADEV_READ_OUT;
            }
            break;
        case 4://k37a user
            {
                    struct user_info user_info;
                    get_current_user_info(&user_info,k37a_dev->pro);
                    copy_to_user(buf,&user_info,sizeof(struct user_info));
                    ret= sizeof(struct user_info);
                    goto K37ADEV_READ_OUT;
            }
            break;
        case 5://k37a get mcu log
            {
                    char log_buff[DATA_MAX_LENGTH]={0};
                    int log_len;
					char order;

					copy_from_user(&order,buf,1);
                    ret=get_mcu_log(&order,log_buff,&log_len,1000,k37a_dev->pro);
                    if(ret!=0){
						ret=-1; 
						goto K37ADEV_READ_OUT;
					}     
                    copy_to_user(buf,log_buff,log_len);
                  
                    ret=log_len;                    
            }
        break;
        case 6://sync mcu time
        break;//no used
        
        case 7://k37a internal temperature
        {
                int k37a_temperature;
                k37a_temperature=read_k37a_temperature(k37a_dev->pro);
                copy_to_user(buf,&k37a_temperature,sizeof(k37a_temperature));
                ret= sizeof(k37a_temperature);
                goto K37ADEV_READ_OUT;
        }
        break;       
        case 8://k37a battery
        {
                char k37a_battery_quantity;
                k37a_battery_quantity=read_battery_quantity(k37a_dev->pro);
                copy_to_user(buf,&k37a_battery_quantity,sizeof(k37a_battery_quantity));
                ret= sizeof(k37a_battery_quantity);
                goto K37ADEV_READ_OUT;
        }
		case 11: //why k37a reboot ？
		{
			
			uint8_t  k37a_reboot[7]; 
			ret = read_reboot_reason(k37a_reboot,k37a_dev->pro);	
			copy_to_user(buf,k37a_reboot,sizeof(k37a_reboot));//lyl
		}
        default:;
    }

K37ADEV_READ_OUT:

    up(&k37a_dev->sem);

    return ret;

    
}


static ssize_t
 k37adev_write(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct  k37adev *k37a_dev=NULL;
    int val;
    int ret=0;
    
    k37a_dev=filp->private_data;
    if(!k37a_dev) 
    {
                printk("k37a_dev NULL porinter\n");
                return -1;
    }

    down(&k37a_dev->sem);
    switch(k37a_dev->chan)
    {
        case 0://users information
            {
                    struct user_info_list user_list;
                    copy_from_user(&user_list,buf,sizeof(struct user_info_list));
                    if(user_list.user_count>=0 && user_list.user_count<=32)
                    {
                            ret=sync_user_infomation(user_list.user_count,user_list.user_info,k37a_dev->pro);
                            if(ret!=0) 
                            {
                                ret= -1;
                                goto K37ADEV_WRITE_OUT;
                            }
                            else
                            {
                                ret= sizeof(struct user_info_list);
                                goto K37ADEV_WRITE_OUT;
                            }
                    }
                    else 
                    {
                        ret= 0;
                        goto K37ADEV_WRITE_OUT;
                    }
            }
            break;
        case 1://ibutton code
            break;//no used
        case 2://k37a door
            break;//no used
        case 3://k37a locker
            {// 1=open 0=close
                      char locker_status;
                      
                      copy_from_user(&locker_status,buf,sizeof(locker_status));
                      if(locker_status<0 || locker_status>1) {ret= 0;goto K37ADEV_WRITE_OUT;}
                      ret=set_k37a_locker(locker_status,k37a_dev->pro);
                      if(ret!=0){ret= -1;goto K37ADEV_WRITE_OUT;}
                      else {ret= sizeof(locker_status);goto K37ADEV_WRITE_OUT;}
              }
        break;
        case 4://k37a user
        case 5://k37a get mcu log
            break;//no used
        case 6://sync mcu time
              {
                    long seconds=0;
                    copy_from_user(&seconds,buf,sizeof(long));
                    ret=sync_mcu_time(seconds,k37a_dev->pro);
                    if(ret!=0){ret= -1;goto K37ADEV_WRITE_OUT;}
                    else {ret= sizeof(long);goto K37ADEV_WRITE_OUT;}
              }
              break;
        case 7://k37a internal temperature
        break;//no used
        case 8://k37a battery
        break;//no used      

		case 9: //update
			{

			struct firmware_packet one_packet;
			struct transfer_result results = {0};
			int recv_len;

			if (count > DATA_MAX_LENGTH) {
				ret= -1;
				goto K37ADEV_WRITE_OUT;
			}
			memset(&one_packet,0,sizeof(struct firmware_packet));
			copy_from_user(&one_packet.order,buf,count);
			ret=transfer_mcu_firmwares(&one_packet,count,&results,&recv_len,k37a_dev->pro);
			if(ret!=0) 
			{
				ret= -1;
				goto K37ADEV_WRITE_OUT;
			}
			else
			{
				//maybe transfer OK ?
				if ((one_packet.order == results.order) && (results.result == 0)) {		
					//printk("packet order = %d transfer OK !\n",one_packet.order);
					//ret= count;
					ret = 0;
				} else {
					//printk("packet order = %d transfer error,errno = %d !\n",one_packet.order,results.result);
					ret = results.result; //return result code to app.
				}
				goto K37ADEV_WRITE_OUT;
			}
		}
		break;
		case 11: //reboot 
		{
			char val = 0;
			ret = copy_from_user(&val,buf,sizeof(buf));
			if(ret) {
				ret= -1;
				goto K37ADEV_WRITE_OUT;
			}
			if(val != 0)
				ret = reboot_device(k37a_dev->pro); //重启设备			
			
		}
              default:;

    }

K37ADEV_WRITE_OUT:
    
    up(&k37a_dev->sem);

    return ret;
}

int export_reboot_func(void)
{

   int ret = 0;
   struct  k37adev *k37a_dev;
   k37a_dev = &k37adev[11]; //reboot index
   ret = reboot_device(k37a_dev->pro); 

   return ret;
}

EXPORT_SYMBOL(export_reboot_func);

static int  k37adev_open(struct inode *inode, struct file *filp)
{
    struct  k37adev *k37a_dev=NULL;
     int dev=0;
     int i=0;
     
     for(i=0;i<K37ADEV_NUM;i++)
    {
            if(k37adev[i].devt == inode->i_rdev)
             {
                        k37a_dev = &k37adev[i];
                        break;
             }
    }

    if(k37a_dev)
    {
            k37a_dev->users++;
            filp->private_data =  k37a_dev;
    }
    return 0;
}

static long k37adev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
        struct  k37adev *k37a_dev=NULL;
        int ret=0;
          
        k37a_dev=filp->private_data;
        if(!k37a_dev) 
        {
                    printk("k37a_dev NULL porinter\n");
                    return -1;
        }
        
        down(&k37a_dev->sem);  
        switch(cmd)
        {
                case CLR_K37A_MCU_LOG :
                {
                        ret=clear_mcu_log(k37a_dev->pro);
                        if(ret!=0)
                        {
                            ret=-1;
                            goto K37ADEV_IOCTL_ERROR;
                        }
                }
                break;
                case GET_K37A_POWER_SUPPLY:
                {
                        char supply =0;
                        supply=read_power_supply(k37a_dev->pro);
                        copy_to_user((char*)arg,&supply,sizeof(supply));
                }
                break;
                case GET_K37A_BATTERY_CHARGE:
                {
                        char charge = 0;
                        charge=read_battry_charge(k37a_dev->pro);
                        copy_to_user((char*)arg,&charge,sizeof(charge));         
                }
                break;            
                case CUT_OFF_BATTERY_ALL_POWER_SUPPLY:
                {
                        ret=cut_off_battery_supply_power_all(k37a_dev->pro);
                        if(ret!=0)
                        {
                            ret=-1;
                            goto K37ADEV_IOCTL_ERROR;
                        }                    
                }
                break;
				case START_MCU_UPDATE:
				{
				    ret=start_mcu_firmware_update((struct firmware_update *)arg,k37a_dev->pro);
                    if(ret!=0)
                    {
                        ret=-1;
                        goto K37ADEV_IOCTL_ERROR;
                    }
				}
				break;

				case USE_NEW_MCU_FIRMWARE:
				{
					char result;
					int length = 0;
					
				    ret=set_mcu_use_new_firmware(&result,&length,k37a_dev->pro);
                    if((ret!=0) || (length == 0))
                    {
                        ret=-1;
                        goto K37ADEV_IOCTL_ERROR;
                    }
					ret = result;
				}
				break;
				// new add
				case BATTERY_GET_VOLT_VALUE :// source battery voltage value.
	            {
					struct battery_voltage bat_vol;
					
					memset(&bat_vol,0,sizeof(struct battery_voltage));
	                ret=get_battery_voltage(&bat_vol,k37a_dev->pro);
	                if(ret!=0)
	                {
	                    ret=-1;
	                    goto K37ADEV_IOCTL_ERROR;
	                }
					copy_to_user((char *)arg,&bat_vol,sizeof(struct battery_voltage));
	            }
	            break;

				case BATTERY_SET_VOLT_CAL :
	            {
					struct battery_calibrate bat_calValue;

				    memset(&bat_calValue,0,sizeof(struct battery_calibrate));
				    copy_from_user(&bat_calValue,(char *)arg,sizeof(struct battery_calibrate));
					ret=sync_battery_calibrates(&bat_calValue,k37a_dev->pro);
	                if(ret!=0)
	                {
	                    ret=-1;
	                    goto K37ADEV_IOCTL_ERROR;
	                }
	            }
	            break;

				case BATTERY_GET_VOLT_CAL :
	            {
					struct battery_calibrate bat_calValue;
					
					memset(&bat_calValue,0,sizeof(struct battery_calibrate));
	                ret=get_battery_calibrates(&bat_calValue,k37a_dev->pro);
	                if(ret!=0)
	                {
	                    ret=-1;
	                    goto K37ADEV_IOCTL_ERROR;
	                }
					copy_to_user((char *)arg,&bat_calValue,sizeof(struct battery_calibrate));
	            }
	            break;

				case MCU_MEMORY_TEST:
				{
					int mem_type=0;
					
					copy_from_user(&mem_type,(char *)arg,sizeof(int));
					ret=start_mcu_memory_test(mem_type,k37a_dev->pro);
					if(ret!=0)
					{
						ret=-1;
						goto K37ADEV_IOCTL_ERROR;
					}
				}
				break;
				
				case GET_MCU_MEMORY_TEST_RESULT:
				{
					struct memory_result mem_res;

					memset(&mem_res,0,sizeof(struct memory_result));
					copy_from_user(&mem_res,(char *)arg,sizeof(struct memory_result));
					ret=get_mcu_memory_test_result(mem_res.memory_type,&mem_res,k37a_dev->pro);
					if(ret!=0)
					{
						ret=-1;
						goto K37ADEV_IOCTL_ERROR;
					}
					copy_to_user((char *)arg,&mem_res,sizeof(struct memory_result));
				}
				break;
	
            default : 
				ret=-1;
				break;
        }
    
K37ADEV_IOCTL_ERROR:
        up(&k37a_dev->sem);
        return ret;

}


static int k37a_dev_probe(struct platform_device *pdev)
{
            struct protocol_data * pro= dev_get_platdata(&pdev->dev);
            struct  k37adev *k37a_dev;
            char dev_name[100]={0};
            int i,ret;
                    
            k37adev=k37a_dev=kmalloc(sizeof(struct  k37adev)*K37ADEV_NUM,GFP_KERNEL);
            memset(k37a_dev,0,sizeof(struct  k37adev)*K37ADEV_NUM);
            
            for(i=0;i<K37ADEV_NUM;i++)
            {
                    k37a_dev[i].pro=pro;
                    sema_init(&k37a_dev[i].sem,1);
                    k37a_dev[i].chan=i;
                    k37a_dev[i].parent = &pdev->dev;
                    k37a_dev[i].devt = MKDEV(K37A_DEV_MAJOR, i);
                    sprintf(dev_name,"k37adev_%s",k37adev_str[i]);
                    k37a_dev[i].dev = device_create(k37adev_class, k37a_dev[i].parent, k37a_dev[i].devt,&k37a_dev[i], dev_name);
                    ret = IS_ERR(k37a_dev[i].dev ) ? PTR_ERR(k37a_dev[i].dev ) : 0;	
                    if(ret!=0) break;
            }
            
	return ret;
}

static const struct file_operations k37adev_fops = {
	.owner =	THIS_MODULE,
	.read =		k37adev_read,
	.write =		k37adev_write,
	.open =		k37adev_open,
	.unlocked_ioctl =k37adev_ioctl, 
};
static struct platform_driver k37a_dev_driver={
           .probe=k37a_dev_probe,
	.driver={
		.name = "k37a-dev",
		.owner = THIS_MODULE,
	},
};

static int __init k37a_init(void)
{
        int status;
        status = register_chrdev(K37A_DEV_MAJOR, "k37adev", &k37adev_fops);
        if (status < 0)
       {
            printk("register k37a device  errror\n");
            return status;
        }
        k37adev_class = class_create(THIS_MODULE, "k37adev");
        if (IS_ERR(k37adev_class)) {
                    unregister_chrdev(K37A_DEV_MAJOR," k37adev");
                    return PTR_ERR(k37adev_class);
        }
        platform_driver_register(&k37a_dev_driver);

        return 0;
}
module_init(k37a_init);

static void __exit k37a_exit(void)
{
        platform_driver_unregister(&k37a_dev_driver);
        class_destroy(k37adev_class);
        unregister_chrdev(K37A_DEV_MAJOR," k37adev");
}
module_exit(k37a_exit);

MODULE_LICENSE("GPL");

