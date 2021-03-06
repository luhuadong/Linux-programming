#ifndef __COMM_COMMAND__
#define __COMM_COMMAND__

//#define COMMAND(x) ((x)&(~(0x7<<5))) //mileszhang
#define COMMAND(x)                  ((x) & 0x1fff)
#define CMD_FLAG(x)                 ((x) & ~0x1fff)
#define CMD_LOW(x)                  ((x) & 0xff)
#define CMD_HIGH(x)                 (((x)>>8) & 0xff)

#define CMD_REQ                     (0x1<<13)
#define CMD_RES                     (0x2<<13)
#define CMD_ACK                     (0x3<<13) 
#define CMD_NAK                     (0x4<<13) 
#define REQUEST(x)                  (CMD_REQ | COMMAND(x) )
#define RESPOND(x)                  (CMD_RES | COMMAND(x) )
#define ACK(x)                      (CMD_ACK | COMMAND(x) )
#define NAK(x)                      (CMD_NAK | COMMAND(x) )

#define GET_DATA_CONF               COMMAND(0x1)
#define SET_DATA_DO_VOLTAGE         COMMAND(0x2)
#define SET_DATA_DO_PULSE           COMMAND(0x3)
#define GET_DATA_DI_COUNTER         COMMAND(0x4)
#define CLR_DATA_DI_COUNTER         COMMAND(0x5)
#define SET_DATA_AI_TYPE            COMMAND(0x6)
#define SYNC_AI_CALIBRATION         COMMAND(0x07)
#define GET_AI_CALIBRATION          COMMAND(0x08)

#define SYNC_USER_INFO              COMMAND(0x09)
#define GET_IBUTTON_CODE            COMMAND(0x10)
#define SYNC_MCU_TIME               COMMAND(0x11)
#define SET_K37A_LOCKER             COMMAND(0x12)
#define GET_MCU_LOG                 COMMAND(0x13)
#define CLR_MCU_LOG                 COMMAND(0x14)
#define CUT_OFF_ALL_POWER_SUPPLY    COMMAND(0x15)
//MCU firmware update command
#define GET_MCU_DEV_INFOS           COMMAND(0x16)
#define START_MCU_FIRMWARE_UPDATE   COMMAND(0x17)
#define START_PACKET_TRANSFER       COMMAND(0x18)
#define USE_NEW_FIRMWARE            COMMAND(0x19)

//new add
#define GET_BATTERY_VOLTAGE         COMMAND(0x20)
#define SYNC_BATTERY_CALIBRATION    COMMAND(0x21)
#define GET_BATTERY_CALIBRATION     COMMAND(0x22)
#define START_MCU_MEM_TEST          COMMAND(0x23)
#define GET_MCU_MEM_TEST_RESULT     COMMAND(0x24)

#define SET_MCU_ANALOG_MODE         COMMAND(0x27)
#define GET_MCU_ANALOG_VALUE        COMMAND(0x28)

#define REQUEST_MCU_REBOOT          COMMAND(0x33)
#define GET_REBOOT_REASON           COMMAND(0x34)


#define ERROR(x)                    (x)
#define ERR_IGNORE                  ERROR(0x1)
#define ERR_TIMEOUT                 ERROR(0x2)
#define ERR_NOT_READY               ERROR(0x3)
#define ERR_ARG_INCOMPLETE          ERROR(0x4)

#define ACK_PERIOD                  500 /*ms*/
#define DATA_PERIOD                 500 /*ms*/
#define SERIAL_SEND_PERIOD          1000/*ms*/
#define SERIAL_RECV_PERIOD          500//1000/*ms*/
#define GET_IBUTTON_CODE_TIMEOUT    1000/*ms*/
#endif
