#!/bin/bash

#boot
#I210 detected
#eth0 okay
#eth1 okay
#dsp connected
#stm32 connected
#err_count=0

FILE="/home/root/.count.txt"

POWER_ON=`grep -o "boot" $FILE | wc -l`
ALL_OKAY=`grep -o "err_count=0" $FILE | wc -l`
I210_OKAY=`grep -o "I210 detected" $FILE | wc -l`
ETH0_OKAY=`grep -o "eth0 okay" $FILE | wc -l`
ETH1_OKAY=`grep -o "eth1 okay" $FILE | wc -l`
DSP_CONN=`grep -o "dsp connected" $FILE | wc -l`
STM32_CONN=`grep -o "stm32 connected" $FILE | wc -l`


function shell_output()
{
	echo "######## Report ########"
	echo "power on: $POWER_ON"
	echo "all okay: $ALL_OKAY"
	echo "i210 yes: $I210_OKAY"
	echo "eth0 yes: $ETH0_OKAY"
	echo "eth1 yes: $ETH1_OKAY"
	echo "connect to dsp: $DSP_CONN"
	echo "connect to stm32: $STM32_CONN"
	
	if [ $ALL_OKAY -ne $POWER_ON ]; then
		echo "------------------------"
		grep "error" $FILE
	fi

	echo "------------------------"
}

function generate_html()
{
	HTML_FILE="/tmp/report.html"

	if [ -f $HTML_FILE ]; then
		rm $HTML_FILE
	fi
	
	echo "<!DOCTYPE html>" >> $HTML_FILE
	echo "<html>" >> $HTML_FILE
	echo "<head>" >> $HTML_FILE
	echo "<meta charset="utf-8">" >> $HTML_FILE
	echo "<title>GYT Workbench</title>" >> $HTML_FILE
	echo "<body>" >> $HTML_FILE
	echo "<h3>Report</h3>" >> $HTML_FILE
	echo "<table border=\"1\"; bordercolor=\"Indigo\" cellspacing=\"0\"; cellpadding=\"4\">" >> $HTML_FILE
	#-------- Header --------
	echo "<tr style=\"background-color:Indigo; color:Lavender\">" >> $HTML_FILE
	echo "<th width=\"128px\">Item</th>" >> $HTML_FILE
	echo "<th width=\"128px\">Count</th>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row1 --------
	echo "<tr>" >> $HTML_FILE
	echo "<td>power on</td>" >> $HTML_FILE
	echo "<td align=\"right\">$POWER_ON</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row2 --------
	echo "<tr style=\"background-color:Lavender\">" >> $HTML_FILE
	echo "<td>all okay</td>" >> $HTML_FILE
	echo "<td align=\"right\">$ALL_OKAY</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row3 --------
	echo "<tr>" >> $HTML_FILE
	echo "<td>i210 yes</td>" >> $HTML_FILE
	echo "<td align=\"right\">$I210_OKAY</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row4 --------
	echo "<tr style=\"background-color:Lavender\">" >> $HTML_FILE
	echo "<td>eth0 yes</td>" >> $HTML_FILE
	echo "<td align=\"right\">$ETH0_OKAY</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row5 --------
	echo "<tr>" >> $HTML_FILE
	echo "<td>eth1 yes</td>" >> $HTML_FILE
	echo "<td align=\"right\">$ETH1_OKAY</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row6 --------
	echo "<tr style=\"background-color:Lavender\">" >> $HTML_FILE
	echo "<td>connect to dsp</td>" >> $HTML_FILE
	echo "<td align=\"right\">$DSP_CONN</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	#-------- Row7 --------
	echo "<tr>" >> $HTML_FILE
	echo "<td>connect to stm32</td>" >> $HTML_FILE
	echo "<td align=\"right\">$STM32_CONN</td>" >> $HTML_FILE
	echo "</tr>" >> $HTML_FILE
	echo "</table>" >> $HTML_FILE
	#-------- The end --------
	echo "<h3>Error</h3>" >> $HTML_FILE

	if [ $ALL_OKAY -ne $POWER_ON ]; then
		echo "`grep \"error\" $FILE | sed \":a;N;s/\n/<br>/g;ba\"`" >> $HTML_FILE
	else
		echo "<p>&nbsp;&nbsp;&nbsp;&nbsp;Null</p>" >> $HTML_FILE
	fi
	echo "</body>" >> $HTML_FILE
	echo "</html>" >> $HTML_FILE
	
	echo "Generate html finished."
	echo "Please type \"file:///tmp/report.html\" on the browser."
	echo "If in command line, maybe you can type:"
	echo "export DISPLAY=:0.0"
	echo "/usr/bin/qt4/demos/browser/browser file:///tmp/report.html"
}


shell_output
generate_html

