#ifndef __PROTOCOL__
#define __PROTOCOL__

#include "commands.h"

#define HEADER_POS 0
#define LEN_POS 3
#define CMD_POS 5
#define DEV_POS 7
#define ERR_POS 7
#define DATA_POS 8
#define STAT_POS 7
#define DI_COUNT_POS 8
//#define RECV_LEN_POS 8
#define RECV_DATA_POS 10
#define IBTN_CODE_POS 7
#define LOG_DATA_POS 7
#define CTRLLER_DATA_POS 9
#define TRANSFER_RESULT_POS 7
#define AI_CALIBRATION_POS  11
#define BATTERY_CALIBRATION_POS     7

static unsigned short get_len(const char * const packet)
{
    unsigned short len=0;

    if(!packet) return -1;
    
    len = packet[LEN_POS]<<8;
    len += packet[LEN_POS+1];
    return len;
}

static unsigned short get_log_len(const char * const packet)
{
    unsigned short len=0;

    if(!packet) return -1;
    
    len = packet[LOG_DATA_POS-2]<<8;
    len += packet[LOG_DATA_POS+1-2];
    return len;
}

static unsigned short get_cmd(const char * const packet)
{
    unsigned short  gcmd=0;

    if(!packet) return -1;

    gcmd = (packet[CMD_POS]<<8)&0xff00;
    gcmd += packet[CMD_POS+1]&0x00ff;

    return gcmd;
}

static unsigned char get_dev(const char * const  packet)
{
    unsigned char dev;

    if(!packet) return -1;

    dev=packet[DEV_POS];

    return dev;
}

static unsigned char get_error(const char * const packet)
{
    unsigned char err;

    if(!packet) return -1;

    err=packet[ERR_POS];

    return err;
}

static unsigned short get_CRC(const char * const packet,int len)
{
    unsigned short crc=0;
    unsigned short tmp=0;
    
    if(!packet) return -1;
    
    crc = packet[len-1] ;
    tmp = packet[len-2];
    crc&=0xff;
    tmp&=0xff;
    crc |= tmp<<8; 
    return crc;
}

static unsigned short CRC16_212(char* buf, int len)
{
    unsigned short  r;
    unsigned char hi;
    char flag;
    int i, j;

    r = 0xffff;
    for(j=0;j<len;j++)
    {
        hi = r >> 8;
        hi ^= buf[j];
        r = hi;

        for(i=0;i<8;i++)
        {
          flag = r & 0x0001;
          r = r >> 1;
          if(flag == 1) r ^= 0xa001;
      }
  }
  return r;
}

static int correct_packet(char *packet,int len)
{
    unsigned short CRC1,CRC2;

    CRC1=get_CRC(packet,len);
    CRC2 = CRC16_212(packet,len-2);

    if(CRC1 != CRC2)
        return -1;
    else
        return 0;
}

#endif
