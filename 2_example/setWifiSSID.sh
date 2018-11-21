#!/bin/bash
echo "################################################################################"
echo "全サンプルスケッチの SSID と PASS を 自動的に設定します。"
echo "書き換えた SSID と PASS を修正する場合は、resetで戻してから、yesで変更して下さい"
echo "                                             Copyright (c) 2016-2019 Wataru KUNINO"
echo "################################################################################"
#         1234567890123 456789012 3
SSID_old="#define SSID \"1234ABCD\"                     // 無線LANアクセスポイントのSSID"
PASS_old="#define PASS \"password\"                     // パスワード"

echo -n "「yes」を入力するとSSIDとPASSを書き換えます。(yes/no/reset) ? "
read yes
if [ "$yes" = "yes" ];then
	echo -n "SSID = "; read SSID
	echo -n "PASS = "; read PASS
	sed -i -e "s/${SSID_old::23}/${SSID_old::14}${SSID}\"/g" -e "s/${PASS_old::23}/${PASS_old::14}${PASS}\"/g" */example*.ino
fi
if [ "$yes" = "reset" ];then
	sed -i -e "/${SSID_old::14}/c${SSID_old}" -e "/${PASS_old::14}/c${PASS_old}" */example*.ino
fi
ls */example*.ino | while read file_ino; do
	echo -n $file_ino" "
	grep "${SSID_old::13}" $file_ino|cut -d" " -f1-3|tr -d "\n"
	echo -n ","
	grep "${PASS_old::13}" $file_ino|cut -d" " -f3
	if [ $? != 0 ];then echo; fi
done
