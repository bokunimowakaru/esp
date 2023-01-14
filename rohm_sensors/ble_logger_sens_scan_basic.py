#!/usr/bin/env python3
# coding: utf-8

################################################################################
# BLE Logger
#
# for:
#   iot_exp_press_ble
#   iot_exp_press_udp_ble
#   iot_exp_sensorShield_ble
#   iot_exp_sensorShield_ble_rh
#   iot_exp_sensorShield_udp_ble
#
# iot_exp_press_ble や iot_exp_sensorShield_ble が送信するビーコンを受信し
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
#   https://www.rohm.co.jp/documents/11401/3946483/sensormedal-evk-002_ug-j.pdf

from bluepy import btle

def payval(num, bytes=1, sign=False):
    global val
    a = 0
    for i in range(0, bytes):
        a += (256 ** i) * int(val[(num - 2 + i) * 2 : (num - 1 + i) * 2],16)
    if sign:
        if a >= 2 ** (bytes * 8):
            a -= 2 ** (bytes * 8)
    return a

def printval(dict, name, n, unit):
    value = dict.get(name)
    if value == None:
        return
    if type(value) is not str:
        if n == 0:
            value = round(value)
        else:
            value = round(value,n)
    print('    ' + name + ' ' * (14 - len(name)) + '=', value, unit)

scanner = btle.Scanner()
while True:
    devices = scanner.scan(3)
    sensors = dict()
    for dev in devices:
        print("\nDevice",dev.addr,"("+dev.addrType+"), RSSI="+str(dev.rssi)+" dB")
        isRohmMedal = ''
        val = ''
        for (adtype, desc, value) in dev.getScanData():
            print("  %3d %s = %s" % (adtype, desc, value))
            if adtype == 9 and value[0:7] == 'espRohm':
                isRohmMedal = 'Sensor Kit espRohm'
            if desc == 'Manufacturer':
                val = value
            if isRohmMedal == '' or val == '':
                continue
            sensors = dict()

            if isRohmMedal == 'Sensor Kit espRohm' and len(val) < 17 * 2:
                sensors['ID'] = hex(payval(2,2))
                sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                sensors['Pressure'] = payval(6,3) / 2048
                sensors['SEQ'] = payval(9)
                sensors['RSSI'] = dev.rssi

            if isRohmMedal == 'Sensor Kit espRohm' and len(val) >= 17 * 2:
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

            if sensors:
                printval(sensors, 'ID', 0, '')
                printval(sensors, 'SEQ', 0, '')
                printval(sensors, 'Temperature', 2, '℃')
                printval(sensors, 'Pressure', 3, 'hPa')
                printval(sensors, 'Illuminance', 1, 'lx')
                printval(sensors, 'Proximity', 0, 'count')
                if(sensors.get('Color R')):
                    print('    Color RGB     =',round(sensors['Color R']),\
                                                round(sensors['Color G']),\
                                                round(sensors['Color B']),'%')
                    print('    Color IR      =',round(sensors['Color IR']),'%')
                printval(sensors, 'Accelerometer', 3, 'g')
                printval(sensors, 'Geomagnetic', 1, 'uT')
                printval(sensors, 'RSSI', 0, 'dB')
            isRohmMedal = ''

''' 実行結果の一例

pi@raspberrypi:~ $ cd
pi@raspberrypi:~ $ git clone http://github.com/bokunimowakaru/rohm_iot_sensor_shield
pi@raspberrypi:~ $ cd ~/rohm_iot_sensor_shield
pi@raspberrypi:~/rohm_iot_sensor_shield $ sudo ./ble_logger_sens_scan_basic.py

Device xx:xx:xx:xx:xx:xx (public), RSSI=-55 dB
    1 Flags = 06
    9 Complete Local Name = espRohm
  255 Manufacturer = 0100b1e9c00001308147ff0041efbbabfa
    ID            = 0x1
    SEQ           = 250
    Temperature   = 29.25 ℃
    Pressure      = 1004 hPa
    Illuminance   = 160.0 lx
    Proximity     = 1 count
    Color RGB     = 19 28 50 %
    Color IR      = 3 %
    Accelerometer = 1.016 g
    Geomagnetic   = 110.8 uT
    RSSI          = -55 dB

Device xx:xx:xx:xx:xx:xx (public), RSSI=-45 dB
    1 Flags = 06
    9 Complete Local Name = espRohmPress
  255 Manufacturer = 0100b76dc45e1f4c
    ID            = 0x1
    SEQ           = 76
    Temperature   = 30.0 ℃
    Pressure      = 1003.846 hPa
    RSSI          = -45 dB
'''
