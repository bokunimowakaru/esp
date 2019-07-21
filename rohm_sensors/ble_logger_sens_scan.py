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
            if adtype == 9 and val[0:7] == 'espRohm':
                isRohmMedal = True
            if isRohmMedal and desc == 'Manufacturer':
                if len(val) >= 17 * 2:
                    # センサ値を辞書型変数sensorsへ代入
                    sensors['ID'] = hex(payval(2,2))
                    sensors['Temperature'] = payval(4,1) / 4 - 15
                    sensors['Pressure'] = payval(5,1,True) + 1027
                    sensors['Illuminance'] = payval(6,2) / 1.2
                    sensors['Proximity'] = payval(8,1)
                    sensors['Color R'] = payval(9,1) / 256 * 100
                    sensors['Color B'] = payval(10,1) / 256 * 100
                    sensors['Color G'] = payval(11,1) / 256 * 100
                    sensors['Color IR'] = 100 - sensors['Color R']\
                                              - sensors['Color G']\
                                              - sensors['Color B']
                    if sensors['Color IR'] < 0:
                        sensors['Color IR'] = 0
                    sensors['Accelerometer X'] = payval(12,1,True) / 64
                    sensors['Accelerometer Y'] = payval(13,1,True) / 64
                    sensors['Accelerometer Z'] = payval(14,1,True) / 64
                    sensors['Accelerometer'] = (sensors['Accelerometer X'] ** 2\
                                              + sensors['Accelerometer Y'] ** 2\
                                              + sensors['Accelerometer Z'] ** 2) ** 0.5
                    sensors['Geomagnetic X'] = payval(15,1,True)
                    sensors['Geomagnetic Y'] = payval(16,1,True)
                    sensors['Geomagnetic Z'] = payval(17,1,True)
                    sensors['Geomagnetic']  = (sensors['Geomagnetic X'] ** 2\
                                             + sensors['Geomagnetic Y'] ** 2\
                                             + sensors['Geomagnetic Z'] ** 2) ** 0.5

                    sensors['SEQ'] = payval(18)
                    sensors['RSSI'] = dev.rssi

                    # 画面へ表示
                    print('    ID            =',sensors['ID'])
                    print('    SEQ           =',sensors['SEQ'])
                    print('    Temperature   =',round(sensors['Temperature'],1),'℃')
                    print('    Pressure      =',round(sensors['Pressure']),'hPa')
                    print('    Proximity     =',round(sensors['Proximity']),'count')
                    print('    Illuminance   =',round(sensors['Illuminance']),'lx')
                    print('    Color RGB     =',round(sensors['Color R']),\
                                                round(sensors['Color G']),\
                                                round(sensors['Color B']),'%')
                    print('    Color IR      =',round(sensors['Color IR']),'%')
                    print('    Accelerometer =',round(sensors['Accelerometer'],3),'g (',\
                                                round(sensors['Accelerometer X'],3),\
                                                round(sensors['Accelerometer Y'],3),\
                                                round(sensors['Accelerometer Z'],3),'g)')
                    print('    Geomagnetic   =',round(sensors['Geomagnetic'],1),'uT (',\
                                                round(sensors['Geomagnetic X']),\
                                                round(sensors['Geomagnetic Y']),\
                                                round(sensors['Geomagnetic Z']),'uT)')
                    print('    RSSI          =',sensors['RSSI'],'dB')
                else:
                    # センサ値を辞書型変数sensorsへ代入
                    sensors['ID'] = hex(payval(2,2))
                    sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                    sensors['Pressure'] = payval(6,3) / 2048
                    sensors['SEQ'] = payval(9)
                    sensors['RSSI'] = dev.rssi

                    # 画面へ表示
                    print('    ID            =',sensors['ID'])
                    print('    SEQ           =',sensors['SEQ'])
                    print('    Temperature   =',round(sensors['Temperature'],2),'℃')
                    print('    Pressure      =',round(sensors['Pressure'],3),'hPa')
                    print('    RSSI          =',sensors['RSSI'],'dB')
