/*================================================================
 *   Copyright (C) 2018 Guangzhou Firefly Ltd. All rights reserved.
 *   
 *   文件名称：modbus_rtu_test.c
 *   创 建 者：luhuadong
 *   创建日期：2018年04月07日
 *   描    述：Linux下modbusRTU测试程序
 *
 ================================================================*/

#include <stdio.h>
#include <stdlib.h>
//#include "modbus.h"
#include <modbus/modbus.h>

int main(void)
{
	modbus_t *mb;
	uint16_t tab_reg[32]={0};
	//open port
	mb = modbus_new_rtu("/dev/ttySAC2",19200,'N',8,1);
	//set slave address
	modbus_set_slave(mb,1);

	modbus_connect(mb);

	struct timeval t;
	t.tv_sec=0;
	//set modbus time 1000ms
	t.tv_usec=1000000;
	modbus_set_response_timeout(mb,&t);

	int regs=modbus_read_registers(mb, 0, 20, tab_reg); 

	printf("%d %d %d %d %d\n", \
			regs,tab_reg[0],tab_reg[1],tab_reg[2],tab_reg[3]);

	modbus_close(mb);  
	modbus_free(mb);
	return 0;
}

