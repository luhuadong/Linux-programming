#!/bin/bash

cp ./count.sh /home/root/
cp ./report.sh /home/root/

#lineNum=`sed -n -e '/exit 0/=' /etc/rc.local`
#lineNum=`echo $lineNum | awk -F " " '{print $NF}'`

line=`sed -n '/exit 0/=' /etc/rc.local  | tail -n1`
echo ${line}
sed -i "${line}i/home/root/count.sh &" /etc/rc.local

echo "GYT Workbench installed, it will be run while reboot."
exit 0
