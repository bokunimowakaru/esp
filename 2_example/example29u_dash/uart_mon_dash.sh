#!/bin/bash
# シリアルUARTからMACアドレスを受信し、リストに一致した機器のログを保存する
# Copyright (c) 2018 Wataru KUNINO

#UART="/dev/ttyUSB0"
UART=""
SAVETO="log_adash_1.csv"                            # ローカルへ
DEVICE="adash_1,"
MAC_LIST=(
    "01:23:45:67:89:AB dash1"
    "00:11:22:33:44:55 dash2"
    "66:77:88:99:AA:BB dash3"
    "CC:DD:EE:FF:00:11 iphone1"
    "22:33:44:55:66:77 iphone7"
)                                                   # 監視対象のMACアドレスと機器名
MAC_NUM=${#MAC_LIST[*]}

UARTS=(`ls /dev/ttyUSB*`) &> /dev/null
if [ ${#UARTS[*]} -lt 1 ]; then
    echo "no UART devices"
    exit
fi
for ((i=0; i < ${#UARTS[*]}; i++)); do
    if [ "$UART" = "${UARTS[$i]}" ]; then
        break
    fi
done
if [ $i -eq ${#UARTS[*]} ]; then
    i=$(( i - 1 ))
    UART=${UARTS[$i]}
fi
echo "UART["${i}"]="${UART}
stty --file ${UART} 115200 igncr
while true; do                                      # 永久に繰り返し
    UIN=`timeout 1 cat /dev/ttyUSB1`
    delimiter=`echo $UIN|cut -d" " -f1`
    mac=`echo $UIN|cut -d" " -f2`
    if [ "${delimiter}" = "'" ] && [ -n $mac ]; then
        DATE=`date "+%Y/%m/%d %R"`
        echo -n ${DATE}", "${mac}" "
        for (( i=0; i < $MAC_NUM; i++ )); do
            mac_array=(${MAC_LIST[$i]})
            mac_address=(${mac_array[0]})
            mac_name=(${mac_array[1]})
            if [ "${mac_address}" = "${mac}" ]; then
                echo $mac_name
                mac_csv=`echo ${mac_address}|tr ":" ","`
                n=$(( i + 1 ))
                echo ${DATE}", "${DEVICE}${n}","${mac_csv}","${mac_name} >> $SAVETO
                break
            fi
        done
        if [ $i -eq $MAC_NUM ]; then
            echo
        fi
    fi
done
