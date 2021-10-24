#!/bin/bash
# LAN内の機器にPINGを打って情報を収集する
# ~/.maclist.txtにMACアドレスとデバイス名(タブ区切り)の対応表を保存しておく
# 類似の方法として nbtscan も便利
# 
# Copyright (c) 2016-2019 Wataru KUNINO

IP=`hostname -I|tr " " "\n"|grep -Eo '([0-9]*\.){3}[0-9]*'|grep -v "127.0."|head -1|cut -d. -f1-3`
OS=`uname`
i=1
while [ ${i} -le 254 ]; do
	# echo ${IP}.${i}
	if [ "$OS" = "Linux" ]; then
		sudo ping -c1 -W1 ${IP}.${i}|grep "time="|awk '{ printf("%s\n",$4)}'|tr -d ':'|while read in; do
			mac=`arp ${in}|tail -1|awk '{ printf("%s",$3)}'`
			echo ${in} ${mac} `grep -i ${mac} ~/.maclist.txt|cut -f2-`
		done &
		sleep 0.01
		PID=`pidof ping` &> /dev/null
		if [ -n "$PID" ];then
			sudo kill $PID
		fi
	else
		sudo ping -n 1 -w 100 ${IP}.${i}|grep -a 'ms TTL='
	fi
	(( i++ ))
done
exit 0
