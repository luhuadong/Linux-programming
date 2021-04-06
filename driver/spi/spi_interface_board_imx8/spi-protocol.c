/*
 * Simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2006 SWAPP
 *  Andrea Paterniani <a.paterniani@swapp-eng.it>
 * Copyright (C) 2007 David Brownell (simplification, cleanup)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <asm/uaccess.h>
#include <linux/gpio.h>

#include "commands.h"
#include "protocol.h"
#include "spi-protocol.h"


static struct protocol_data  protocol;
static char *protocol_name = "prodata";
struct mcu_dev_infos _g_mcu_describes = {0};
static int board_infos_ok = 0;
/*-------------------------------------------------------------------------*/
static int protocol_engine(struct protocol_data *,pack_str_t *);
static int host_recv_packet(struct protocol_data *pro,char *buf,int *len,unsigned long timeout);

int reset_reboot_count(struct protocol_data * pro)
{
    if(!pro) return 1;
    
    pro->reboot_count = 0;
    return 0;
}

float  get_ai(int pos,struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    float  ai;
    
    pos=(pos-1)%_g_mcu_describes.ai_num;
    down(&mcu_conf->sem);
    ai=mcu_conf->ai[pos].f_value;
    up(&mcu_conf->sem);

    return  ai;
}

unsigned int get_di(int pos,struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf = &pro->mcu_conf;
    unsigned int di;

    pos=(pos-1)%_g_mcu_describes.di_num;
    down(&mcu_conf->sem);
    di=mcu_conf->di[pos];
    up(&mcu_conf->sem);
    return  di;
}

unsigned int get_do(int pos,struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    unsigned int ddo=0;
    
    pos=(pos-1)%_g_mcu_describes.do_num;
    down(&mcu_conf->sem);
    ddo=mcu_conf->ddo[pos];
    up(&mcu_conf->sem);
    return  ddo;
}

int  set_do_voltage(unsigned char  dev, int val,struct protocol_data * pro)
{
    //struct protocol_data *pro=&protocol;
    pack_str_t  pack_str;
    unsigned short pos , arg;
    int ret;
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    
    if(dev>_g_mcu_describes.do_num || dev<1) return -1;
    
    pos = 1<<(dev-1);
    arg=val ? 1:0;
    arg=arg<<(dev-1);
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SET_DATA_DO_VOLTAGE);
    pack_str.byte_8th.dodi_dev=pos;
    pack_str.arg = (char *)&arg;
    pack_str.arg_len=sizeof(arg);
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    if (ret == 0) {
        pos=(dev-1)%_g_mcu_describes.do_num;
        mcu_conf->ddo[pos] = arg;
    }
    
    return ret;
}

int set_do_pulse(unsigned char  dev,unsigned short millisecond,struct protocol_data * pro)
{
    //struct protocol_data *pro=&protocol;
    pack_str_t  pack_str;
    unsigned short pos ;
    unsigned short msecond[8]={0};
    int ret;
    
    if(dev>_g_mcu_describes.do_num || dev<1) return -1;
    
    pos =1<<(dev-1);
    msecond[_g_mcu_describes.do_num-dev] = htons(millisecond);

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SET_DATA_DO_PULSE);
    pack_str.byte_8th.dodi_dev=pos;
    pack_str.arg = (char *)msecond;
    pack_str.arg_len=sizeof(unsigned short)*_g_mcu_describes.do_num;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int get_di_counters(unsigned char dev,unsigned long *counter,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int len=RES_CMD_COUNTER_DATA_LEN;
    unsigned long buf[RES_CMD_COUNTER_DATA_LEN/4]={0};
    unsigned short pos;
    int ret=0;

    if(dev>_g_mcu_describes.di_num || dev<1) return -1;
    
    pos = 1<<(dev-1);

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_DATA_DI_COUNTER);
    pack_str.byte_8th.dodi_dev=pos;
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data = (char *)buf;
    pack_str.data_len =&len;
    pack_str.timeout = DATA_PERIOD;
    
    if(protocol_engine(pro,&pack_str)==0)
    {
     *counter=htonl(buf[dev-1]);
     ret = 0;
 }
 else 
    ret=-1;

return ret;
}

int clear_di_counters(unsigned char dev,struct protocol_data * pro)
{
    //struct protocol_data *pro=&protocol;
    pack_str_t  pack_str;
    unsigned short pos;
    int ret;
    
    if(dev>_g_mcu_describes.di_num || dev<1) return -1;
    pos = 1<<(dev-1);
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(CLR_DATA_DI_COUNTER);
    pack_str.byte_8th.dodi_dev=pos;
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int set_ai_type(unsigned int dev,unsigned short type,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    unsigned int pos;
    int ret;
    int index1,index2;
    char arg[8];

    if(dev > _g_mcu_describes.ai_num || dev < 1) return -1;
    pos = 1 << (dev - 1);

    index1 = (dev - 1) / 4;
    index2 = (dev - 1) % 4;
    memset(arg,0,sizeof(arg));
    arg[index1] |= (type << (index2*2));
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SET_DATA_AI_TYPE);
    pack_str.byte_8th.ai_dev=pos;
    pack_str.arg = (char *)&arg;
    pack_str.arg_len=8;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int sync_ai_calibrates(unsigned char dev,struct ai_calibrate *ai_calValue,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    unsigned int pos;
    
    if(!ai_calValue) return -1;

    if(dev > _g_mcu_describes.ai_num || dev < 1) return -1;
    pos = 1 << (dev - 1);
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SYNC_AI_CALIBRATION);
    pack_str.byte_8th.ai_dev=pos;
    pack_str.arg = (char *)ai_calValue;
    pack_str.arg_len=sizeof(struct ai_calibrate);
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;

}

int get_ai_calibrates(unsigned char dev,struct ai_calibrate *ai_calValue,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;     
    unsigned int pos;

    if(!ai_calValue) return -1;

    if(dev > _g_mcu_describes.ai_num || dev < 1) return -1;
    pos = 1 << (dev-1);

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_AI_CALIBRATION);
    pack_str.byte_8th.ai_dev=pos;
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data = (char *)ai_calValue;
    pack_str.data_len =&length;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int sync_user_infomation(int user_count,struct user_info *user_info,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    char user_code_list[256]={0};
    unsigned char count;
    int i=0;
    int ret;
    
    if(!user_info) return -1;
    
    count = user_count;
    for(i=0;i<count;i++)
        memcpy(&user_code_list[i*8],user_info[i].code,8);
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SYNC_USER_INFO);
    pack_str.byte_8th.user_count=count;
    pack_str.arg=user_code_list;
    pack_str.arg_len=8*count;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;

}

int get_current_user_info(struct user_info *user_info,struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    int ret=0;
    if(!user_info) return -1;

    down(&mcu_conf->sem);
    user_info->user= mcu_conf->user[0];//  1:known user,  0: unknown user
    memcpy(user_info->code,&mcu_conf->user[1],8);
    up(&mcu_conf->sem);

    return  ret;
}

int get_ibutton_code(struct user_info *user_info,unsigned long timeout,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int code_len = sizeof(user_info->code);
    int ret;
    
    if(!user_info) return -1;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_IBUTTON_CODE);
    pack_str.arg=NULL;
    pack_str.arg_len=0;
    pack_str.data=user_info->code;
    pack_str.data_len =&code_len;
    pack_str.timeout = timeout;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

char read_k37a_door(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    unsigned char k37a_door;

    down(&mcu_conf->sem);
    k37a_door= mcu_conf->k37a_door;
    up(&mcu_conf->sem);

    return  k37a_door;
}

int sync_mcu_time(long seconds,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    char tm[4]={0};
    int ret;

    tm[0]=seconds>>24 &0xff;
    tm[1]=seconds>>16 &0xff;
    tm[2]=seconds>>8 & 0xff;
    tm[3]=seconds & 0xff;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SYNC_MCU_TIME);
    pack_str.arg=tm;
    pack_str.arg_len=4;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

char read_k37a_locker(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    unsigned char k37a_locker;

    down(&mcu_conf->sem);
    k37a_locker= mcu_conf->k37a_locker;
    up(&mcu_conf->sem);

    return  k37a_locker;
}

int set_k37a_locker(char locker,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

/*
    if(locker)
        printk("\nopen k37a locker\n");
    else
        printk("\nclose k37a locker\n"); 
*/
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SET_K37A_LOCKER);
    pack_str.arg=&locker;
    pack_str.arg_len=1;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int read_k37a_temperature(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    int temperature;

    down(&mcu_conf->sem);
    temperature = mcu_conf->k37a_temperature;
    temperature -= 100;
    up(&mcu_conf->sem);
    /*printk("temperature : %d,temp2:%d\n",temperature,mcu_conf->k37a_temperature);*/

    return  temperature;
}

int get_mcu_log(char *order,char *log,int *log_len,unsigned long timeout,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_MCU_LOG);
    pack_str.arg=order;
    pack_str.arg_len=1;
    pack_str.data=log;
    pack_str.data_len =log_len;
    pack_str.timeout = timeout;
    ret=protocol_engine(pro,&pack_str);

    //printk("\nkernel : get mcu log: len(%d) ,%s,ret=%d\n",*log_len,log,ret);
    
    return ret;
}

int clear_mcu_log(struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(CLR_MCU_LOG);
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

/*
 * return %0~%100
 */
char read_battery_quantity(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    char quantity;

    down(&mcu_conf->sem);
    quantity= mcu_conf->battery & 0x3f;
    quantity=(quantity*100)>>6; // linux kernel no float cal 
    up(&mcu_conf->sem);
    //printk("quantity=%d\n",quantity);
    return  quantity;//0~100
}

char read_power_supply(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    unsigned char supply;

    down(&mcu_conf->sem);
    supply= mcu_conf->battery & 0x80;
    up(&mcu_conf->sem);
    
    //printk("supply=%d\n",supply);
    
    return  supply>0 ? 1 : 0;
}

char read_battry_charge(struct protocol_data * pro)
{
    struct mcu_conf_t *mcu_conf=&pro->mcu_conf;
    unsigned char charge;

    down(&mcu_conf->sem);
    charge= mcu_conf->battery & 0x40;
    up(&mcu_conf->sem);

    return  charge>0 ? 1 : 0;
}

int cut_off_battery_supply_power_all(struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(CUT_OFF_ALL_POWER_SUPPLY);
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

/* new add */
int start_mcu_firmware_update(struct firmware_update *datas,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(START_MCU_FIRMWARE_UPDATE);
    pack_str.byte_8th.order = datas->total;
    pack_str.arg=datas->cur_version;
    pack_str.arg_len=sizeof(struct firmware_update) - 2;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = SERIAL_SEND_PERIOD*3;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int transfer_mcu_firmwares(struct firmware_packet *datas,int datas_len,
    struct transfer_result *result,int *result_len,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;    
    
    if(!datas || !result) return -1;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(START_PACKET_TRANSFER);
    pack_str.byte_8th.order = datas->order;
    pack_str.arg = datas->datas;
    pack_str.arg_len=datas_len - 2;
    pack_str.data = (char *)result;
    pack_str.data_len =result_len;
    pack_str.timeout = SERIAL_SEND_PERIOD*3;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int set_mcu_use_new_firmware(char *data,int * data_len,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(USE_NEW_FIRMWARE);
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data=data;
    pack_str.data_len = data_len;
    pack_str.timeout = SERIAL_SEND_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int get_mcu_dev_infos(struct mcu_dev_infos *mcu_dev_desc,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_MCU_DEV_INFOS);
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data = (char *)mcu_dev_desc;
    pack_str.data_len = &length;
    pack_str.timeout = GET_IBUTTON_CODE_TIMEOUT;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

//new add
int get_battery_voltage(struct battery_voltage *bat_vol,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_BATTERY_VOLTAGE);
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data = (char *)bat_vol;
    pack_str.data_len = &length;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

int sync_battery_calibrates(struct battery_calibrate *bat_cal,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    
    if(!bat_cal) return -1;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SYNC_BATTERY_CALIBRATION);
    pack_str.arg = (char *)bat_cal;
    pack_str.arg_len=sizeof(struct battery_calibrate);
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int get_battery_calibrates(struct battery_calibrate *bat_cal,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;     

    if(!bat_cal) return -1;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_BATTERY_CALIBRATION);
    pack_str.arg=NULL;
    pack_str.arg_len=0;
    pack_str.data = (char *)bat_cal;
    pack_str.data_len =&length;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int start_mcu_memory_test(int memory_type,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(START_MCU_MEM_TEST);
    pack_str.byte_8th.memory_type=memory_type;
    pack_str.arg=NULL;
    pack_str.arg_len=0;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int get_mcu_memory_test_result(int memory_type,struct memory_result *mem_res,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_MCU_MEM_TEST_RESULT);
    pack_str.byte_8th.memory_type=memory_type;
    pack_str.arg=NULL;
    pack_str.arg_len=0;
    pack_str.data = (char *)mem_res;
    pack_str.data_len =&length;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

//new add for tester
int set_mcu_analog_mode(int analog_mode,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;

    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(SET_MCU_ANALOG_MODE);
    pack_str.byte_8th.memory_type=analog_mode;// use memory_type to store it for tester channal.
    pack_str.arg=NULL;
    pack_str.arg_len=0;
    pack_str.data=0;
    pack_str.data_len =0;
    pack_str.timeout = ACK_PERIOD;
    ret=protocol_engine(pro,&pack_str);
    
    return ret;
}

int get_mcu_analog_value(unsigned char dev,struct analog_value *analog_vol,struct protocol_data * pro)
{
    pack_str_t  pack_str;
    int ret;
    int length;
    unsigned int pos;
    
    if(dev > _g_mcu_describes.ai_num || dev < 1) return -1;
    pos = dev-1;
    
    memset(&pack_str,0,sizeof(pack_str_t));
    pack_str.cmd=REQUEST(GET_MCU_ANALOG_VALUE);
    pack_str.byte_8th.memory_type=pos;  // use memory_type to store it for tester channal.
    pack_str.arg=0;
    pack_str.arg_len=0;
    pack_str.data = (char *)analog_vol;
    pack_str.data_len = &length;
    pack_str.timeout = SERIAL_SEND_PERIOD*2;
    ret=protocol_engine(pro,&pack_str);

    return ret;
}

//end

static int create_packet(char * packet,pack_str_t * pack_str)
{    
    unsigned short cmd;
    void * arguments;
    int arg_len;
    unsigned short CRC;
    int pos;

    if(!pack_str || !packet)     return -1;

    cmd=pack_str->cmd;
    arguments=pack_str->arg;
    arg_len=pack_str->arg_len;

    memset(packet,0,CMD_LENGTH+arg_len);
    memcpy(packet,"#0#",3);
    pos=5;
    packet[pos++] = CMD_HIGH(cmd);
    packet[pos++] = CMD_LOW(cmd);
    //printk("## current cmd = %02x ##\n",COMMAND(cmd));

    switch(COMMAND(cmd))
    {
        case GET_DATA_CONF:
        case GET_IBUTTON_CODE:
        case  SYNC_MCU_TIME:
        case SET_K37A_LOCKER:  
        case GET_MCU_LOG:
        case CLR_MCU_LOG:
        case GET_BATTERY_VOLTAGE:
        case GET_BATTERY_CALIBRATION:   
        break;
        
        case SET_DATA_DO_VOLTAGE: 
        case SET_DATA_DO_PULSE:    
        case GET_DATA_DI_COUNTER:
        case CLR_DATA_DI_COUNTER :
        {        
            unsigned short dodi_dev;
            dodi_dev=pack_str->byte_8th.dodi_dev;
            packet[pos++] = (dodi_dev >> 8) & 0xff;
            packet[pos++] = dodi_dev&0xff;
        }
        break;
        case START_MCU_MEM_TEST:
        case GET_MCU_MEM_TEST_RESULT:
        case SET_MCU_ANALOG_MODE:
        case GET_MCU_ANALOG_VALUE:
        {        

            packet[pos++]=pack_str->byte_8th.memory_type;
        }
        break;  
        case SET_DATA_AI_TYPE:
        case SYNC_AI_CALIBRATION:
        case GET_AI_CALIBRATION:
        {        
            unsigned int ai_dev;
            ai_dev = pack_str->byte_8th.ai_dev;
            packet[pos++] = (ai_dev >> 24) & 0xff;
            packet[pos++] = (ai_dev >> 16) & 0xff;
            packet[pos++] = (ai_dev >> 8) & 0xff;
            packet[pos++] = ai_dev & 0xff;
        }
        break;

        case  SYNC_USER_INFO:
        {
            char user_count;
            user_count = pack_str->byte_8th.user_count;
            packet[pos++]=user_count;
        }
        break;
        case  START_MCU_FIRMWARE_UPDATE:
        case  START_PACKET_TRANSFER:
        {
            unsigned short order;
        order = pack_str->byte_8th.order;   //this total packet in START_MCU_FIRMWARE_UPDATE CMD.
        packet[pos++] = (order >> 8) & 0xff;
        packet[pos++] = order&0xff;
    }
    break;
    default : ;      

}

if(arg_len!=0 )
{
    switch(COMMAND(cmd))
    {
        case SET_DATA_DO_VOLTAGE: 
        {        
            char *data = (char *)arguments;
            packet[pos++] = *(data + 1);
            packet[pos++] = *data;
        }
        break;
        case SYNC_AI_CALIBRATION:
        {
                //vol_a
            char *data = (char *)arguments;
            packet[pos++] = *(data + 3);
            packet[pos++] = *(data + 2);
            packet[pos++] = *(data + 1);
            packet[pos++] = *(data);
                //vol_b
            packet[pos++] = *(data + 7);
            packet[pos++] = *(data + 6);
            packet[pos++] = *(data + 5);
            packet[pos++] = *(data + 4);
                //cur_a
            packet[pos++] = *(data + 11);
            packet[pos++] = *(data + 10);
            packet[pos++] = *(data + 9);
            packet[pos++] = *(data + 8);
                //cur_b
            packet[pos++] = *(data + 15);
            packet[pos++] = *(data + 14);
            packet[pos++] = *(data + 13);
            packet[pos++] = *(data + 12);
                //CRC
            packet[pos++] = *(data + 17);
            packet[pos++] = *(data + 16);
        }
        break;
        case SYNC_BATTERY_CALIBRATION:
        {
                //vol_a
            char *data = (char *)arguments;
            packet[pos++] = *(data + 3);
            packet[pos++] = *(data + 2);
            packet[pos++] = *(data + 1);
            packet[pos++] = *(data);
                //vol_b
            packet[pos++] = *(data + 7);
            packet[pos++] = *(data + 6);
            packet[pos++] = *(data + 5);
            packet[pos++] = *(data + 4);
                //CRC
            packet[pos++] = *(data + 9);
            packet[pos++] = *(data + 8);
        }
        break;
        case START_MCU_FIRMWARE_UPDATE:
        {
            char *data = (char *)arguments;
                memcpy(&packet[pos],arguments,arg_len - 2); //version
                pos += (arg_len - 2);
                packet[pos++] = data[arg_len - 1];
                packet[pos++] = data[arg_len - 2];
            }
            break;
            default:
            {
                memcpy(&packet[pos],arguments,arg_len);
                pos+=arg_len;
            }
            break;
        }
    }
    
    /*such packet total length*/
    packet[3] = ((pos+2)>>8)&0xff;
    packet[4] = (pos+2)&0xff;    
    
    CRC = CRC16_212(packet,pos);
    packet[pos++] = (CRC >> 8)&0xff;
    packet[pos++] = CRC&0xff;
    
    return pos;
}


static void spipro_complete(void *arg)
{
    complete(arg);
}

static ssize_t
spipro_sync(struct protocol_data *pro, struct spi_message *message)
{
    DECLARE_COMPLETION_ONSTACK(done);
    int status;
    
    message->complete = spipro_complete;
    message->context = &done;

    spin_lock_irq(&pro->spi_lock);
    if (pro->spidev == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_async(pro->spidev, message);
    spin_unlock_irq(&pro->spi_lock);

    if (status == 0) {
        wait_for_completion(&done);
        status = message->status;
        if (status == 0)
            status = message->actual_length;
    }
    return status;
}

ssize_t spipro_sync_write(struct protocol_data *pro, char *buffer, size_t len)
{
    struct spi_transfer t = {
        .tx_buf = buffer,
        .len    = len,
    };
    struct spi_message  m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    return spipro_sync(pro, &m);
}

ssize_t spipro_sync_read(struct protocol_data *pro, char *buffer, size_t len)
{
    struct spi_transfer t = {
        .rx_buf = buffer,
        .len    = len,
    };
    struct spi_message  m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    return spipro_sync(pro, &m);
}

static int protocol_send_packet(struct protocol_data *pro,char *packet, int len)
{
    int size = 0;
    int ret =0;
    char sen_buff[len + 10]; /*reserved 10 Bytes*/
    
    if((!packet) || (len <= 0) || (!pro->spidev)) 
        return FLAG_ERROR;

    print_hex("send protocol packet",packet,len);
    memset(sen_buff, 0, sizeof(sen_buff));
    mutex_lock(&pro->spidev_lock);
    memcpy(sen_buff, packet, len);
    size = spipro_sync_write(pro, sen_buff, len);
    mutex_unlock(&pro->spidev_lock);
    if(size<=0)
    {
        printk("spi send data error size=%d\n",size);
        return FLAG_ERROR;
    }
    
    return ret;
}

static int protocol_recv_packet(struct protocol_data *pro,char *packet, int *size,unsigned long timeout)
{
    char *buf = packet;
    int len = 0;
    int ret = 0;
    
    ret = host_recv_packet(pro, buf, &len, timeout);
    if(ret < 0)
        *size = 0;
    else
        *size = len;

    return ret;
}

#define CACHE_LEN (RES_CMD_CONF_DATA_LEN+14)    
static int exist_header(const char *buf)
{
    int i=0;
    for(i=0;i<CACHE_LEN-5;i++)/*header 3+length 2*/
    if(!memcmp(&buf[i],"#0#",3)) return i; 

    return -1;
}

static int host_recv_packet(struct protocol_data *pro,char *buf,int *len,unsigned long timeout)
{
    char cache[CACHE_LEN] = {0};
    char recv_buff[CACHE_LEN] = {0};
    int pack_len = 0, size = 0, diff = 0, pos = 0;
    int ret = 0;

    mutex_lock(&pro->spidev_lock);
    size = spipro_sync_read(pro, recv_buff, CACHE_LEN);
    if(size <= CACHE_LEN && size > 0)
        memcpy(cache, recv_buff, sizeof(cache)); // only get CACHE_LEN data to analyse received packet,which maybe include another packet
    mutex_unlock(&pro->spidev_lock);
    print_hex("host receive  cache", cache, sizeof(cache));   

    if((pos = exist_header(cache))<0) {
        DBG_ERR("error : host received cache is not a packet\n");
        *len = 0;
        ret = -1;
        goto PACK_RECV_FAILED;        
    }

    pack_len = get_len(&cache[pos]);
    if(pack_len > PACK_MAX_LENGTH) {
        *len = 0;
        ret = -1;
        goto PACK_RECV_FAILED;
    }

    diff = CACHE_LEN - pos;
    memcpy(buf, &cache[pos], diff);

    if(pack_len > diff) {
        mutex_lock(&pro->spidev_lock);
        size = spipro_sync_read(pro, recv_buff, pack_len - diff);
        if(size >0 ) memcpy(&buf[diff], recv_buff, size);
        mutex_unlock(&pro->spidev_lock);
    }

    if(pack_len > (size + diff))
    {
        DBG_ERR("host received packet have more data to receive\n");
        *len=0;
        ret=-1;
        goto PACK_RECV_FAILED;
    }    

    ret = correct_packet(buf, pack_len);
    if(ret != 0) {
        DBG_ERR("host received packet is CRC error\n");
        *len = 0;
        ret = -1;
        goto PACK_RECV_FAILED;
    }              

    *len = pack_len;
    ret = 0;
    
    PACK_RECV_FAILED:    
    //if(ret < 0)        
    //    err_hex("receive  packet error",buf,pack_len>0 ? pack_len : size);

    return ret;
}

static int analyse_packet(struct protocol_data *pro,char *packet,int len,char *recv_packet,int recv_len,pack_str_t * pack_str)
{
    char *data; 
    int *data_len;
    unsigned short cmd,rcmd;
    int ret=0;

    if(!pro ||!packet ||!recv_packet||!pack_str) return -1;
    if(len==0 ||recv_len==0 ) return -1;

    cmd=get_cmd(packet);
    rcmd=get_cmd(recv_packet);

    if(COMMAND(cmd) != COMMAND(rcmd))
    {
        DBG_ERR("request and respond command not match : %x,%x\n",cmd,rcmd);
        return -1;
    }
    if(rcmd == NAK(rcmd))
    {
        ret=get_error(recv_packet);
        return ret;
    }

    data = pack_str->data;
    data_len=pack_str->data_len;

    switch(rcmd)
    {
        case ACK(SET_DATA_DO_VOLTAGE):
        case ACK(SET_DATA_DO_PULSE):
        case ACK(CLR_DATA_DI_COUNTER):
        case ACK(SET_DATA_AI_TYPE):
        case ACK(SYNC_USER_INFO):
        case ACK(SYNC_MCU_TIME):
        case ACK(SET_K37A_LOCKER):
        case ACK(CLR_MCU_LOG):
        case ACK(CUT_OFF_ALL_POWER_SUPPLY):
                /* new add  */
        case ACK(SYNC_AI_CALIBRATION):
        case ACK(START_MCU_FIRMWARE_UPDATE):
        case ACK(SYNC_BATTERY_CALIBRATION):
        case ACK(START_MCU_MEM_TEST):   
        case ACK(SET_MCU_ANALOG_MODE):

        case RESPOND(SET_DATA_DO_VOLTAGE):
        case RESPOND(SET_DATA_DO_PULSE):
        case RESPOND(CLR_DATA_DI_COUNTER):
        case RESPOND(SET_DATA_AI_TYPE):
        case RESPOND(SYNC_USER_INFO):
        case RESPOND(SYNC_MCU_TIME):
        case RESPOND(SET_K37A_LOCKER):
        case RESPOND(CLR_MCU_LOG):                    
        case RESPOND(CUT_OFF_ALL_POWER_SUPPLY):
        case RESPOND(SYNC_BATTERY_CALIBRATION):
        case RESPOND(START_MCU_MEM_TEST):
        case RESPOND(SET_MCU_ANALOG_MODE):
        break;

        case RESPOND(GET_DATA_CONF) :
        {                    
            int pack_len=get_len(recv_packet);

            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            memcpy(data,&recv_packet[STAT_POS],RES_CMD_CONF_DATA_LEN);
            *data_len = RES_CMD_CONF_DATA_LEN;
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_DATA_DI_COUNTER):
        {
            int pack_len=get_len(recv_packet);

            if(!data || !data_len) goto DATA_RECV_FAILED;
            if(get_dev(recv_packet)!=get_dev(packet)) goto DATA_RECV_FAILED;
            if((pack_len-(CMD_BASE_LEN+1))<=0)goto DATA_RECV_FAILED;

            memcpy(data,&recv_packet[DI_COUNT_POS],RES_CMD_COUNTER_DATA_LEN);
            *data_len = RES_CMD_COUNTER_DATA_LEN;
            ret=FLAG_OK;        
        }
        break;

        case RESPOND(GET_IBUTTON_CODE):
        {
            int pack_len=get_len(recv_packet);

            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            memcpy(data,&recv_packet[IBTN_CODE_POS],8);
            *data_len = 8;
            ret=FLAG_OK;
        }
        break;
        case RESPOND(GET_MCU_LOG):
        {
            int pack_len=get_len(recv_packet);
            int log_len;

            if(!data || !data_len) 
                goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)
                goto DATA_RECV_FAILED;

                    log_len = pack_len-LOG_DATA_POS-2;/*2=CRC_len*/
            if((get_log_len(recv_packet)!=log_len) && (log_len>DATA_MAX_LENGTH)) 
                goto DATA_RECV_FAILED;

            memcpy(data,&recv_packet[LOG_DATA_POS],log_len);
            *data_len = log_len;
            ret=FLAG_OK;
        }
        break;

                /* new add */
        case RESPOND(START_PACKET_TRANSFER):      
        {
            int pack_len=get_len(recv_packet);
            struct transfer_result *result = (struct transfer_result *)data;

            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            result->order = recv_packet[TRANSFER_RESULT_POS];
            result->order <<= 8;
            result->order |= recv_packet[TRANSFER_RESULT_POS + 1];
            result->result = recv_packet[TRANSFER_RESULT_POS + 2];
            *data_len = 3;
            ret=FLAG_OK;            
        }
        break;

        case RESPOND(USE_NEW_FIRMWARE):
        {
            int pack_len=get_len(recv_packet);
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;        
            *data = recv_packet[TRANSFER_RESULT_POS];
            *data_len = 1;
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_MCU_DEV_INFOS):
        {
            int pack_len=get_len(recv_packet);
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;    
            memcpy(data,&recv_packet[7],sizeof(struct mcu_dev_infos));
            *data_len = sizeof(struct mcu_dev_infos);
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_AI_CALIBRATION):
        {
            int pack_len=get_len(recv_packet);
            struct ai_calibrate *ai_cals = (struct ai_calibrate *)data;
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;
                    //vol_a
            ai_cals->vol_a.c_value[0] = recv_packet[AI_CALIBRATION_POS + 3];
            ai_cals->vol_a.c_value[1] = recv_packet[AI_CALIBRATION_POS + 2];
            ai_cals->vol_a.c_value[2] = recv_packet[AI_CALIBRATION_POS + 1];
            ai_cals->vol_a.c_value[3] = recv_packet[AI_CALIBRATION_POS];

                    //vol_b
            ai_cals->vol_b.c_value[0] = recv_packet[AI_CALIBRATION_POS + 7];
            ai_cals->vol_b.c_value[1] = recv_packet[AI_CALIBRATION_POS + 6];
            ai_cals->vol_b.c_value[2] = recv_packet[AI_CALIBRATION_POS + 5];
            ai_cals->vol_b.c_value[3] = recv_packet[AI_CALIBRATION_POS + 4];

                    //curr_a
            ai_cals->cur_a.c_value[0] = recv_packet[AI_CALIBRATION_POS + 11];
            ai_cals->cur_a.c_value[1] = recv_packet[AI_CALIBRATION_POS + 10];
            ai_cals->cur_a.c_value[2] = recv_packet[AI_CALIBRATION_POS + 9];
            ai_cals->cur_a.c_value[3] = recv_packet[AI_CALIBRATION_POS + 8];

                    //curr_b
            ai_cals->cur_b.c_value[0] = recv_packet[AI_CALIBRATION_POS + 15];
            ai_cals->cur_b.c_value[1] = recv_packet[AI_CALIBRATION_POS + 14];
            ai_cals->cur_b.c_value[2] = recv_packet[AI_CALIBRATION_POS + 13];
            ai_cals->cur_b.c_value[3] = recv_packet[AI_CALIBRATION_POS + 12];
            ai_cals->CRC = recv_packet[AI_CALIBRATION_POS + 16];
            ai_cals->CRC <<= 8;
            ai_cals->CRC |= recv_packet[AI_CALIBRATION_POS + 17];

            *data_len = sizeof(struct ai_calibrate);
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_BATTERY_VOLTAGE):
        {
            int pack_len=get_len(recv_packet);
            struct battery_voltage *bat_vol = (struct battery_voltage *)data;
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            bat_vol->battery_vol.c_value[0] = recv_packet[BATTERY_CALIBRATION_POS + 3];
            bat_vol->battery_vol.c_value[1] = recv_packet[BATTERY_CALIBRATION_POS + 2];
            bat_vol->battery_vol.c_value[2] = recv_packet[BATTERY_CALIBRATION_POS + 1];
            bat_vol->battery_vol.c_value[3] = recv_packet[BATTERY_CALIBRATION_POS];

            *data_len = sizeof(struct battery_voltage);
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_MCU_ANALOG_VALUE):
        {
            int pack_len=get_len(recv_packet);
            struct analog_value *analog_vol = (struct analog_value *)data;
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            analog_vol->analog_vol.c_value[0] = recv_packet[BATTERY_CALIBRATION_POS + 3];
            analog_vol->analog_vol.c_value[1] = recv_packet[BATTERY_CALIBRATION_POS + 2];
            analog_vol->analog_vol.c_value[2] = recv_packet[BATTERY_CALIBRATION_POS + 1];
            analog_vol->analog_vol.c_value[3] = recv_packet[BATTERY_CALIBRATION_POS];

            *data_len = sizeof(struct analog_value);
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_BATTERY_CALIBRATION):
        {
            int pack_len=get_len(recv_packet);
            struct battery_calibrate *bat_cals = (struct battery_calibrate *)data;
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;
                    //vol_a
            bat_cals->vol_a.c_value[0] = recv_packet[BATTERY_CALIBRATION_POS + 3];
            bat_cals->vol_a.c_value[1] = recv_packet[BATTERY_CALIBRATION_POS + 2];
            bat_cals->vol_a.c_value[2] = recv_packet[BATTERY_CALIBRATION_POS + 1];
            bat_cals->vol_a.c_value[3] = recv_packet[BATTERY_CALIBRATION_POS];

                    //vol_b
            bat_cals->vol_b.c_value[0] = recv_packet[BATTERY_CALIBRATION_POS + 7];
            bat_cals->vol_b.c_value[1] = recv_packet[BATTERY_CALIBRATION_POS + 6];
            bat_cals->vol_b.c_value[2] = recv_packet[BATTERY_CALIBRATION_POS + 5];
            bat_cals->vol_b.c_value[3] = recv_packet[BATTERY_CALIBRATION_POS + 4];
            bat_cals->CRC = recv_packet[BATTERY_CALIBRATION_POS + 8];
            bat_cals->CRC <<= 8;
            bat_cals->CRC |= recv_packet[BATTERY_CALIBRATION_POS + 9];
            *data_len = sizeof(struct battery_calibrate);
            ret=FLAG_OK;
        }
        break;

        case RESPOND(GET_MCU_MEM_TEST_RESULT):
        {
            int pack_len=get_len(recv_packet);
            struct memory_result *result = (struct memory_result *)data;
            if(!data || !data_len) goto DATA_RECV_FAILED;
            if((pack_len-CMD_BASE_LEN)<=0)goto DATA_RECV_FAILED;

            result->memory_type = recv_packet[BATTERY_CALIBRATION_POS];
            result->result = recv_packet[BATTERY_CALIBRATION_POS + 1];
            *data_len = sizeof(struct memory_result);
            ret=FLAG_OK;
        }
        break;

        default : 
        DBG_ERR("command is not existed\n");
        printk("recv command %x is not existed\n",rcmd);
        ret=-1;
        if(data_len) *data_len=0;
    }

    return ret;

    DATA_RECV_FAILED:
    if(data_len)  *data_len=0;
    ret=-1;  
    return ret;
}

static int protocol_engine(struct protocol_data *pro,pack_str_t * pack_str)
{
    int size;
    int recv_size; 
    long time=0;
    int ret=0;
    
    if(!pack_str || !pro) return -1;

    msleep(100);
    
    down(&pro->sem);
    
    size=create_packet(pro->snd_pack,pack_str);
    if(size < CMD_BASE_LEN)
    {
        printk("host protocol create packet error\n");
        ret= -1;
        goto PROTOCOL_ENGINE_ERROR;
    }   
    ret = protocol_send_packet(pro,pro->snd_pack,size);
    if(ret!=0)
    {
        printk("host protocol send do di ai packet error\n");
        ret= -1;
        goto PROTOCOL_ENGINE_ERROR;
    }
    do{
        time+=100;
        msleep(100);

        recv_size = pro->pack_size;
        memset(pro->rcv_pack,0,recv_size);
        ret = protocol_recv_packet(pro,pro->rcv_pack,&recv_size,pack_str->timeout);
        if(ret!=0)
        {
                //printk("host protocol recv packet error\n");
            continue;
        }
#if 0
        printk("===========================PRINT AI===========================\n");
        printk("%s\n", (pro->rcv_pack)+7);
           printk("=============================END================================\n"); 
#endif
       //printk("recv_size=%d\n",recv_size);//receive emtpy rs232 serial packet when recv_size==12
            //if(recv_size==12)print_hex("host receive  packet : ",pro->rcv_pack,12);
        ret=analyse_packet(pro,pro->snd_pack,size,pro->rcv_pack,recv_size,pack_str);
        if(ret!=0)
        {
                //printk("analyse packet error\n");
                //ret>0?printk("wait for ack error : %s\n",info_err[ret]):printk("wait for ack error,not NAK\n");
            ret= -1;
            continue;
        }
    }while(ret!=0 && time<pack_str->timeout);
    
    if(time>=pack_str->timeout)
    {
        ret=-1;
        DBG_ERR("host protocol receive packet error , timeout  error\n");
        goto PROTOCOL_ENGINE_ERROR;
    }
    
    // add 50ms,modify: receive emtpy rs232 serial packet
    msleep(50);
    
    print_hex("host receive  packet : ",pro->rcv_pack,recv_size);

    PROTOCOL_ENGINE_ERROR:
    up(&pro->sem);
    return ret;
}

static int protocol_spi_work(void *work_data)
{
    pack_str_t  pack_str;
    struct protocol_data *pro = (struct protocol_data *)work_data;
    char *data = pro->data;
    int len = pro->data_len;
    struct mcu_conf_t *mcu_conf = &pro->mcu_conf;      
    int ret=0;
    int i=0;
    int cur_pos = 0;
    unsigned short di_val = 0;  
    unsigned short do_val = 0;
    int act_led=0;//ACT-LED

    msleep(1000);

    while(1)
    {
        if (board_infos_ok == 1) {
            memset(&pack_str,0,sizeof(pack_str_t));
            pack_str.cmd=REQUEST(GET_DATA_CONF);
            pack_str.data=data;
            pack_str.data_len =&len;
            pack_str.timeout = DATA_PERIOD;
            ret=protocol_engine(pro,&pack_str);

            if((ret==0)&&(pack_str.data!=NULL) &&( pack_str.data_len !=NULL) )
            {                     
                down(&mcu_conf->sem);
                mcu_conf->rw_flag = data[0];
                    //user information
                mcu_conf->user[0]=data[1];
                memcpy(&mcu_conf->user[1],&data[2],8);
                    //battery info
                mcu_conf->battery = data[10];
                    //k37a door and locker
                mcu_conf->k37a_door = (data[11]>>4) & 0xf;
                mcu_conf->k37a_locker = data[11] & 0xf;
                cur_pos = 12;
                for(i=0;i<_g_mcu_describes.ai_num;i++)
                    {//ai
                        mcu_conf->ai[i].c_value[0] = data[cur_pos + 4*i + 3];
                        mcu_conf->ai[i].c_value[1] = data[cur_pos + 4*i + 2];
                        mcu_conf->ai[i].c_value[2] = data[cur_pos + 4*i + 1];
                        mcu_conf->ai[i].c_value[3] = data[cur_pos + 4*i];
                    }
                    //di
                    cur_pos = 12 + _g_mcu_describes.ai_num * 4;
                    di_val = data[cur_pos];
                    di_val <<= 8;
                    di_val |= data[cur_pos + 1];
                    
                    for(i=0;i<_g_mcu_describes.di_num;i++) {
                        mcu_conf->di[i]=(di_val >> i)&0x1;
                        //printk("di[%i] = %d\n",i,mcu_conf->di[i]);
                    }
                    //do
                    cur_pos += 2;
                    do_val = data[cur_pos];
                    do_val <<= 8;
                    do_val |= data[cur_pos + 1];
                    for(i=0;i<_g_mcu_describes.do_num;i++) {
                        mcu_conf->ddo[i]=(do_val >> i)&0x1;
                        //printk("ddo[%i] = %d\n",i,mcu_conf->ddo[i]);
                    }   
                    up(&mcu_conf->sem);                
                }                
            }       
            msleep(900);/*force schduling process*/
            
            act_led = (act_led+1)%2;
            if(pro->run_gpio)
                gpiod_set_value(pro->run_gpio, act_led);//RUN-LED//added by Max zhang for shining run-led

            if( pro->reboot_count++  > 132 ) { /*复位在新内核暂无这个*/
                  //      extern void (*arch_reset)(char, const char *);
            printk("reboot system!!!!\n");
            msleep(2000);
                  //      arch_reset('h',"vmlinux");                    
        }
    }

    return 0;
}

static struct platform_device mcu_do_dev = {
    .name   = "mcu-do",
    .id = -1,
    .dev    = {
        .platform_data  = &protocol,
    },
};

static struct platform_device mcu_di_dev = {
    .name   = "mcu-di",
    .id = -1,
    .dev    = {
        .platform_data  = &protocol,
    },
};

static struct platform_device mcu_ai_dev = {
    .name   = "mcu-ai",
    .id = -1,
    .dev    = {
        .platform_data  = &protocol,
    },
};

static struct platform_device k37adev_dev = {
    .name   = "k37a-dev",
    .id = -1,
    .dev    = {
        .platform_data  = &protocol,
    },
};

static struct platform_device k37areboot_dev = {
    .name   = "dev_reboot",
    .id = -1,
    .dev    = {
        .platform_data  = &protocol,
    },
};

static int  spipro_probe(struct spi_device *spi)
{
    struct protocol_data *pro = &protocol;
    int ret;
    struct mcu_dev_infos tmp_dev_desc = {0};
    int i;
    
    memset(pro, 0, sizeof(struct protocol_data));
    spin_lock_init(&pro->spi_lock);
    mutex_init(&pro->spidev_lock);
    pro->spidev = spi;

    pro->run_gpio = devm_gpiod_get_optional(&spi->dev, "run", GPIOD_OUT_LOW);
    
    if (IS_ERR(pro->run_gpio)) {
        ret = PTR_ERR(pro->run_gpio);
        dev_err(&spi->dev, "Failed to request run led GPIO: %d\n",
            ret);
        pro->run_gpio = NULL;
    } else {
        printk("get run gpio is ok %d\n",desc_to_gpio(pro->run_gpio));
        gpiod_direction_output(pro->run_gpio, 0 );
    }

    sema_init(&pro->sem,1);
    sema_init(&pro->mcu_conf.sem,1);
    pro->snd_pack = kmalloc(PACK_MAX_LENGTH,GFP_KERNEL);
    pro->rcv_pack = kmalloc(PACK_MAX_LENGTH,GFP_KERNEL);
    pro->pack_size = PACK_MAX_LENGTH;
    pro->data = kmalloc(DATA_MAX_LENGTH,GFP_KERNEL);
    pro->data_len = DATA_MAX_LENGTH;

    strcpy(pro->name, protocol_name);

    pro->task = kthread_create(protocol_spi_work, (void *)pro, protocol_name);
    if(IS_ERR(pro->task))
    {
        printk("create kthread task error\n");
        return 1;
    }
    wake_up_process(pro->task);

    memset(&_g_mcu_describes,0,sizeof(struct mcu_dev_infos));
    sprintf(_g_mcu_describes.hardware_version, "unknown");
    sprintf(_g_mcu_describes.software_version, "unknown");
    // use the mini to init system
    _g_mcu_describes.do_num = MCU_MAX_DO_NUM / 2;
    _g_mcu_describes.di_num = MCU_MAX_DI_NUM / 2;
    _g_mcu_describes.ai_num = MCU_MAX_AI_NUM / 2;
    for (i = 0; i < GET_MAX_TRYS; i++) {
        memset(&tmp_dev_desc,0,sizeof(struct mcu_dev_infos));
        ret = get_mcu_dev_infos(&tmp_dev_desc,pro);
        if (ret != 0) {
            msleep(500);
            continue;
        } else {
            printk("Get interface device infos OK!\n");
            memcpy(&_g_mcu_describes, &tmp_dev_desc, sizeof(struct mcu_dev_infos));         
            break;
        }
    }
    board_infos_ok = 1;
    if (_g_mcu_describes.ai_num > MCU_MAX_AI_NUM) {
        _g_mcu_describes.ai_num = MCU_MAX_AI_NUM / 2;
    }
    if (_g_mcu_describes.do_num > MCU_MAX_DO_NUM) {
        _g_mcu_describes.do_num = MCU_MAX_DO_NUM / 2;
    }
    if (_g_mcu_describes.di_num > MCU_MAX_DI_NUM) {
        _g_mcu_describes.di_num = MCU_MAX_DI_NUM / 2;
    }
    printk("Harware version:%s,Software version:%s,%d DOs,%d DIs and %d AIs will be used.",
        _g_mcu_describes.hardware_version,
        _g_mcu_describes.software_version,
        _g_mcu_describes.do_num,
        _g_mcu_describes.di_num,
        _g_mcu_describes.ai_num);

    platform_device_register(&mcu_do_dev);
    platform_device_register(&mcu_di_dev);
    platform_device_register(&mcu_ai_dev);
    platform_device_register(&k37adev_dev);
    platform_device_register(&k37areboot_dev);

    return 0;
}

static int  spipro_remove(struct spi_device *spi)
{
    platform_device_unregister(&mcu_do_dev);
    platform_device_unregister(&mcu_di_dev);
    platform_device_unregister(&mcu_ai_dev);
    platform_device_unregister(&k37adev_dev);
    platform_device_unregister(&k37areboot_dev);
    
    return 0;
}

static const struct of_device_id spipro_of_match[] = {
    { .compatible = "spi-mcu-protocol" },
    { /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, spipro_of_match);

static struct spi_driver spipro_driver = {
    .probe =  spipro_probe,
    .remove = spipro_remove,
    .driver = {
        .name  = "bcon,spipro",
        .owner = THIS_MODULE,
        .of_match_table = spipro_of_match, 
    },
};

static int __init spipro_init(void)
{
    return spi_register_driver(&spipro_driver);
}

static void __exit spipro_exit(void)
{
    spi_unregister_driver(&spipro_driver);
}

module_init(spipro_init);
module_exit(spipro_exit);

MODULE_AUTHOR("Liyaolong: bocon");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spidev:spi-protocol driver");
