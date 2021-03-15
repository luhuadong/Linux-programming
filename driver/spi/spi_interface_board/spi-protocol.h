#ifndef __SPIPRO_COMMON__
#define __SPIPRO_COMMON__

#define CMD_BASE_LEN 9 //8 //mileszhang
#define ACK_CMD  CMD_BASE_LEN
#define NAK_CMD  (CMD_BASE_LEN+1)
#define CMD_LENGTH 88
#define DATA_MAX_LENGTH 1400
#define PACK_MAX_LENGTH (CMD_LENGTH+DATA_MAX_LENGTH)
#define RES_CMD_CONF_DATA_LEN 87//57
#define RES_CMD_COUNTER_DATA_LEN 32
#define SEND_MAX_LENGTH 512
#define RECV_MAX_LENGTH 1488

#define GET_MAX_TRYS	3
#define MCU_MAX_AI_NUM 	32
#define MCU_MAX_DI_NUM 	16
#define MCU_MAX_DO_NUM 	16

typedef struct cmd_pack_struct {
    unsigned short cmd;
    union {
	    unsigned short dodi_dev;
	    char user_count;
		unsigned short order;
		unsigned int ai_dev;
		unsigned char memory_type;
    } byte_8th;
    int  arg_len;
    char  *arg;
    int  *data_len;
    char *data;
    unsigned int timeout;
    unsigned int dataout;//for later update.
    //if use dataout  , analyse_packet will be wrong for receiveing different packet while not enough data length.
} pack_str_t;

enum{
    FLAG_OK=0,
    FLAG_IGNORE,
    FLAG_TIMEOUT,
    FLAG_NOT_READY,
    FLAG_ARG_INCOMPLETE,
    FLAG_ERROR,
    FLAG_MAX
};

static char * info_err[FLAG_MAX+1]={
    "success",
    "NAK cmd ignore",
    "NAK timeout",
    "NAK not-ready",
    "NAK argument incomplete",
    "NAK error"
};

typedef union {
	float f_value;
	char c_value[4];
} float2char;

struct mcu_dev_infos {
	char hardware_version[30];
	char software_version[30];
	unsigned char interface_num;
	unsigned char do_num;
	unsigned char di_num;
	unsigned char ai_num;
};

struct user_info{
    char user;
    char code[8];
};

struct user_info_list{
    int user_count;
    struct user_info user_info[32];
};

struct firmware_update {
	unsigned short total;
	char cur_version[30];
	unsigned short total_crc;
};

struct firmware_packet {
	unsigned short order; //must put here
	char datas[1024];
};

struct transfer_result {
	unsigned short order;
	char result;
};

struct ai_calibrate {
	float2char vol_a;
	float2char vol_b;
	float2char cur_a;
	float2char cur_b;
	unsigned short CRC;
};

//new add
struct battery_calibrate {
	float2char vol_a;
	float2char vol_b;
	unsigned short CRC;
};

struct battery_voltage {
	float2char battery_vol;
};

struct analog_value {
	float2char analog_vol;
};

struct memory_result {
	int memory_type;
	int result;
};

struct mcu_conf_t{
	char rw_flag;
    char user[9];//  1st byte: =1:known user,  =0: unknown user, others: user code 
    char battery;// 1st bit ,1:power supply 0:power cut off. 2nd battery status,1:charge, 0:not charge , others:%power volumn
    char k37a_door;// 1:open , 0:close
    char k37a_locker;// 1:open , 0:close
    unsigned char k37a_temperature;//0~200 for -100~+100   C
    char room_door; // 1:open, 0:close
    char air_conditioner; // 1:open, 0:close
    char room_temperature;//0~200 for -100~+100   C
    char room_humidity;// 0~100 for 0%~100%
    char room_user[9];//  1st byte: =1:known user,  =0: unknown user, others: user code 
    char room_battery;// 1st bit ,1:power supply 0:power cut off. 2nd battery status,1:charge, 0:not charge , others:%power volumn
    float2char ai[MCU_MAX_AI_NUM];
    int di[MCU_MAX_DI_NUM];
    int ddo[MCU_MAX_DO_NUM];
	struct semaphore sem;
};

struct protocol_data
{
        char name[10];
        char * data;
        int data_len;
        char *snd_pack;
        char *rcv_pack;
        int pack_size;
        struct semaphore sem;
        struct spidev_data	* spidev;
        struct task_struct *task;
        struct mcu_conf_t mcu_conf;

        int reboot_count;
};

//#define DEBUG_COMM
#if DEBUG_COMM
static void print_hex(char *info,unsigned char *data,int len)
{
    int i=0;
    
    if(!data || len==0) return ;
    
    printk("\n%s , data len = %d\n",info,len);
    for(i=0;i<len;i++)
    {
    	printk("%.2x ",data[i]);
        if((i + 1)%40==0)
			printk("\n");     
    }
    printk("\n");
}
#else
#define print_hex(x,y,z) 
#endif


//#define DBG_ERR printk

#define DBG_ERR(arg...)
#ifdef DBG_PRINTK
static void err_hex(char *info,unsigned char *data,int len)
{
    int i=0;
    
    if(!data || len==0) return ;
    
    printk("%s , data len = %d\n",info,len);
    for(i=0;i<len;i++)
    {
        if(i%40==0)printk("\n");
        printk("%.2x ",data[i]);
        if((data[i]==data[(i+1)%len]) &&(data[i]!=0)) printk("() ");
    }
    printk("\n");
}
#else 
#define err_hex(x,y,z) 
#endif

extern struct mcu_dev_infos _g_mcu_describes;


#endif
