#!/usr/bin/env python3
# coding: utf-8

################################################################################
# BLE Logger
#
# iot_exp_press_udp_bleやiot_exp_sensorShield-evk_bleが送信するビーコンを受信し
# ビーコンに含まれる、温度センサ値と気圧センサ値を表示します。
#
#                                               Copyright (c) 2019 Wataru KUNINO
################################################################################

#【インストール方法】
#   bluepy (Bluetooth LE interface for Python)をインストールしてください
#       sudo pip3 install bluepy
#
#【実行方法】
#   実行するときは sudoを付与してください
#       sudo ./ble_logger_sens_scan.py &
#
#【参考文献】
#   本プログラムを作成するにあたり下記を参考にしました
#   https://ianharvey.github.io/bluepy-doc/scanner.html

interval = 1.01 # 動作間隔

from bluepy import btle
from sys import argv
import getpass
from time import sleep

def payval(num, bytes=1, sign=False):
    global val
    a = 0
    if num < 2 or len(val) < (num - 2 + bytes) * 2:
        print('ERROR: data length',len(val))
        return 0
    for i in range(0, bytes):
        a += (256 ** i) * int(val[(num - 2 + i) * 2 : (num - 1 + i) * 2],16)
    if sign:
        if a >= 2 ** (bytes * 8 - 1):
            a -= 2 ** (bytes * 8)
    return a

scanner = btle.Scanner()
while True:
    # BLE受信処理
    try:
        devices = scanner.scan(interval)
    except Exception as e:
        print("ERROR",e)
        if getpass.getuser() != 'root':
            print('使用方法: sudo', argv[0])
            exit()
        sleep(interval)
        continue

    # 受信データについてBLEデバイス毎の処理
    for dev in devices:
        print("\nDevice %s (%s), RSSI=%d dB, Connectable=%s" % (dev.addr, dev.addrType, dev.rssi, dev.connectable))
        isRohmMedal = False
        sensors = dict()
        for (adtype, desc, val) in dev.getScanData():
            print("  %3d %s = %s" % (adtype, desc, val))
            if adtype == 9 and val[0:4] == 'esp_':
                isRohmMedal = True
            if isRohmMedal and desc == 'Manufacturer':

                # センサ値を辞書型変数sensorsへ代入
                sensors['ID'] = hex(payval(2,2))
                sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                sensors['Pressure'] = payval(6,3) / 2048
                sensors['SEQ'] = payval(9)
                sensors['RSSI'] = dev.rssi

                # 画面へ表示
                print('    ID            =',sensors['ID'])
                if sensors['ID'] == '0x179' and sensors['SEQ'] == 15:
                    print('    No Sensors')
                    isRohmMedal = False
                    break
                print('    SEQ           =',sensors['SEQ'])
                print('    Temperature   =',round(sensors['Temperature'],2),'℃')
                print('    Pressure      =',round(sensors['Pressure'],3),'hPa')
                print('    RSSI          =',sensors['RSSI'],'dB')
