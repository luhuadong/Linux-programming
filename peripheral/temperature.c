/*================================================================
*   Copyright (C) 2019 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：temperature.c
*   创 建 者：luhuadong
*   创建日期：2019年02月25日
*   描    述：飞腾1500A处理器CPU温度检测程序，执行时需要root权限
*             
*             输出可能如下：
*             map_base : 7f85be3000
*             data_base: 7f85be3d00
*             ProbeCpuTemperature: 29
*
================================================================*/


#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h> 
#include <sys/mman.h>

#define PHYTIUM1500A_SENSOR_BASE_ADDRESS    0x28100D00
#define PHYTIUM1500A_SENSOR_CTLR            (0)
#define PHYTIUM1500A_SENSOR01_VAL           (0x4)
#define PHYTIUM1500A_SENSOR23_VAL           (0x8)
#define PHYTIUM1500A_SENSOR45_VAL           (0xC)
#define PHYTIUM1500A_SENSOR67_VAL           (0x10)
#define MAP_SIZE        0x20

#define UINT32	unsigned int
#define UINTN	UINT32
#define BOOLEAN	int
#define FALSE	0
#define TRUE	1


/* Convert table of sensor value to temperature */
static int phytium1500a_val2temp[35][2] = {
	{3795, -40000},
	{3787, -35000},
	{3779, -30000},
	{3769, -25000},
	{3761, -20000},
	{3751, -15000},
	{3743, -10000},
	{3733, -5000},
	{3723, 0},
	{3713, 5000},
	{3703, 10000},
	{3693, 15000},
	{3683, 20000},
	{3673, 25000},
	{3663, 30000},
	{3651, 35000},
	{3641, 40000},
	{3629, 45000},
	{3617, 50000},
	{3605, 55000},
	{3593, 60000},
	{3581, 65000},
	{3569, 70000},
	{3555, 75000},
	{3543, 80000},
	{3529, 85000},
	{3515, 90000},
	{3501, 95000},
	{3487, 100000},
	{3471, 105000},
	{3457, 110000},
	{3441, 115000},
	{3425, 120000},
	{3409, 125000},
	{0,    0xFFFFFFFF},
};

static unsigned char *data_base = NULL;

UINT32 MmioRead32(int offsetAddr){
	return *(volatile unsigned int *)(data_base+offsetAddr);
}

void MmioWrite32(int offsetAddr, UINT32 value){
	*(volatile unsigned int *)(data_base+offsetAddr) = value;
}

static int phytium1500a_calc_temprature(int sensor_val)
{
	int index = 0;

	while (phytium1500a_val2temp[index + 1][0] > sensor_val)
	   index ++;

	return phytium1500a_val2temp[index][1];
}

void  EnableTemperatureSensor(void)
{
	UINT32 Sensor_Ctl;

	Sensor_Ctl = MmioRead32(PHYTIUM1500A_SENSOR_CTLR);
	MmioWrite32(PHYTIUM1500A_SENSOR_CTLR, Sensor_Ctl & ~0x80000000);
}

void  DisableTemperatureSensor(void)
{
	UINT32 Sensor_Ctl;
	UINT32 CpuType;

	Sensor_Ctl = MmioRead32(PHYTIUM1500A_SENSOR_CTLR);
	MmioWrite32(PHYTIUM1500A_SENSOR_CTLR, Sensor_Ctl | 0x80000000);
}

UINTN ProbeCpuTemperature(void)
{
	UINTN  TempVal = 0, TempValInCelsius = 0;
	static BOOLEAN SensorEnabled = FALSE;

	if (!SensorEnabled) {
		EnableTemperatureSensor();
		SensorEnabled = TRUE;
	}

	UINT32 Sensor01_Val, Sensor23_Val, Sensor45_Val, Sensor67_Val;

	Sensor01_Val = MmioRead32(PHYTIUM1500A_SENSOR01_VAL);
	Sensor23_Val = MmioRead32(PHYTIUM1500A_SENSOR23_VAL);
	Sensor45_Val = MmioRead32(PHYTIUM1500A_SENSOR45_VAL);
	Sensor67_Val = MmioRead32(PHYTIUM1500A_SENSOR67_VAL);

	TempVal += phytium1500a_calc_temprature((Sensor01_Val) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor01_Val >> 16) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor23_Val) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor23_Val >> 16) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor45_Val) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor45_Val >> 16) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor67_Val) & 0xFFF);
	TempVal += phytium1500a_calc_temprature((Sensor67_Val >> 16) & 0xFFF);

	TempValInCelsius = TempVal / 1000 / 8;

	return TempValInCelsius;
}


void main()
{
	static int dev_fd;
	static unsigned char *map_base = NULL;
	off_t pageSize = getpagesize();
	size_t actualLen = 0;
	off_t actualOffset = 0;
		
	dev_fd = open("/dev/mem", O_RDWR | O_NDELAY);      
 
	if (dev_fd < 0)  
	{
		printf("open(/dev/mem) failed.");    
		return 0;
	}  
	actualOffset = PHYTIUM1500A_SENSOR_BASE_ADDRESS / pageSize * pageSize;
	actualLen    = MAP_SIZE + PHYTIUM1500A_SENSOR_BASE_ADDRESS % pageSize;
	map_base = (unsigned char * )mmap(NULL, actualLen, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, actualOffset);
	if(map_base != NULL){
		data_base = map_base + PHYTIUM1500A_SENSOR_BASE_ADDRESS % pageSize;
		printf("map_base : %lx\n", map_base);
		printf("data_base: %lx\n", data_base);
		printf("ProbeCpuTemperature: %d\n", ProbeCpuTemperature());
	}
 
	if(dev_fd)
		close(dev_fd);
 
	if(map_base != NULL){
		munmap(map_base, MAP_SIZE);//解除映射关系
	}
 
	return 0;
}
