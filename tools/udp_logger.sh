#!/bin/bash
#
# UDPを受信する
#
echo "Listening UDP port "${1}"..."
while [ ${1} -ge 1 ] && [ ${1} -lt 65536 ]
do
	UDP=`sudo netcat -luw0 ${1}`
	DATE=`date "+%Y/%m/%d %R"`
	echo $DATE, $UDP
done
