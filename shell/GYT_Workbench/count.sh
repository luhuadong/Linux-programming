#!/bin/sh

DSP_IP="192.168.3.1"
STM32_IP="192.168.3.188"
FILE="/home/root/.count.txt"

if [ ! -f $FILE ]; then
	touch $FILE
fi

RESULT="boot"
ERR_COUNT=0

FIND_I210=`lspci | grep I210`
if [ $? -ne 0 ]; then
	#ERR_COUNT+=1
	#ERR_COUNT=$ERROR_COUNT+1
	ERR_COUNT=`expr $ERR_COUNT + 1`
	#let ERR_COUNT=ERR_COUNT+1
fi

FIND_ETH0=`ifconfig | grep eth0`
if [ $? -ne 0 ]; then
	ERR_COUNT=`expr $ERR_COUNT + 1`
fi

FIND_ETH1=`ifconfig | grep eth1`
if [ $? -ne 0 ]; then
	ERR_COUNT=`expr $ERR_COUNT + 1`
fi

PING_DSP=`ping -I eth1 -c 1 ${DSP_IP} | grep "1 packets received"`
if [ $? -ne 0 ]; then
	ERR_COUNT=`expr $ERR_COUNT + 1`
fi

PING_STM32=`ping -I eth1 -c 1 ${STM32_IP} | grep "1 packets received"`
if [ $? -ne 0 ]; then
	ERR_COUNT=`expr $ERR_COUNT + 1`
fi

################################

if [ -n "$FIND_I210" ]
then
	RESULT+=",I210 detected"
	#RESULT="$RESULT,I210 detected"
else
	RESULT+=",I210 error"
fi

if [ -n "$FIND_ETH0" ]
then
	RESULT+=",eth0 okay"
else
	RESULT+=",eth0 error"
fi

if [ -n "$FIND_ETH1" ]
then
	RESULT+=",eth1 okay"
else
	RESULT+=",eth1 error"
fi

if [ -n "$PING_DSP" ]
then
	RESULT+=",dsp connected"
else
	RESULT+=",dsp error"
fi

if [ -n "$PING_STM32" ]
then
	RESULT+=",stm32 connected"
else
	RESULT+=",stm32 error"
fi

RESULT+=",err_count=$ERR_COUNT"

echo "$RESULT" >> $FILE

exit 0
