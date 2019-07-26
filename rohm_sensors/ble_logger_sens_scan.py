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

ambient_chid='0000'                 # ここにAmbientで取得したチャネルIDを入力
ambient_wkey='0123456789abcdef'     # ここにはライトキーを入力
ambient_interval = 30               # Ambientへの送信間隔
interval = 1.01                     # 動作間隔
savedata = True                     # ファイル保存の要否
username = 'pi'                     # ファイル保存時の所有者名

from bluepy import btle
from sys import argv
import getpass
from shutil import chown
from time import sleep
import urllib.request                           # HTTP通信ライブラリを組み込む
import json                                     # JSON変換ライブラリを組み込む
import datetime

url_s = 'https://ambidata.io/api/v2/channels/'+ambient_chid+'/data' # アクセス先
head_dict = {'Content-Type':'application/json'} # ヘッダを変数head_dictへ
body_dict = {'writeKey':ambient_wkey, \
            'd1':0, 'd2':0, 'd3':0, 'd4':0, 'd5':0, 'd6':0, 'd7':0, 'd8':0}

def save(filename, data):
    try:
        fp = open(filename, mode='a')                   # 書込用ファイルを開く
    except Exception as e:                              # 例外処理発生時
        print(e)                                        # エラー内容を表示
    fp.write(data + '\n')                               # dataをファイルへ
    fp.close()                                          # ファイルを閉じる
    chown(filename, username, username)                 # 所有者をpiユーザへ

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

def printval(dict, name, n, unit):
    value = dict.get(name)
    if value == None:
        return
    if type(value) is not str:
        if n == 0:
            value = round(value)
        else:
            value = round(value,n)
    print('    ' + name + ' ' * (14 - len(name)) + '=', value, unit, end='')
    if name == 'Accelerometer' or name == 'Geomagnetic':
        print(' (',round(sensors[name + ' X'],n),\
            round(sensors[name + ' Y'],n),\
            round(sensors[name + ' Z'],n), unit + ')')
    else:
        print()

scanner = btle.Scanner()
time = 999
isMedalAvail = False
if ambient_interval < 30:
    ambient_interval = 30

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
    sensors = dict()

    # 受信データについてBLEデバイス毎の処理
    for dev in devices:
        print("\nDevice %s (%s), RSSI=%d dB, Connectable=%s" % (dev.addr, dev.addrType, dev.rssi, dev.connectable))
        isRohmMedal = ''
        val = ''
        for (adtype, desc, value) in dev.getScanData():
            print("  %3d %s = %s" % (adtype, desc, value))
            if desc == 'Short Local Name' and value[0:10] == 'ROHMMedal2':
                isRohmMedal = 'Sensor Medal'
            if adtype == 9 and value[0:7] == 'espRohm':
                isRohmMedal = 'Sensor Kit espRohm'
            if desc == 'Complete Local Name' and value == 'R':
                isRohmMedal = 'Sensor Kit RH'
            if adtype == 8 and value[0:10] == 'LapisDev':
                isRohmMedal = 'Spresense Rohm IoT'
            if desc == 'Manufacturer':
                val = value
            if isRohmMedal == '' or val == '':
                continue
            sensors = dict()
            print('    isRohmMedal   =',isRohmMedal)

            if isRohmMedal == 'Sensor Medal':
                # センサ値を辞書型変数sensorsへ代入
                sensors['ID'] = hex(payval(2,2))
                sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                sensors['Humidity'] = 100 * payval(6,2) / 65536
                sensors['SEQ'] = payval(8)
                sensors['Condition Flags'] = bin(int(val[16:18],16))
                sensors['Accelerometer X'] = payval(10,2,True) / 4096
                sensors['Accelerometer Y'] = payval(12,2,True) / 4096
                sensors['Accelerometer Z'] = payval(14,2,True) / 4096
                sensors['Accelerometer'] = sensors['Accelerometer X']\
                                         + sensors['Accelerometer Y']\
                                         + sensors['Accelerometer Z']
                sensors['Geomagnetic X'] = payval(16,2,True) / 10
                sensors['Geomagnetic Y'] = payval(18,2,True) / 10
                sensors['Geomagnetic Z'] = payval(20,2,True) / 10
                sensors['Geomagnetic']   = sensors['Geomagnetic X']\
                                         + sensors['Geomagnetic Y']\
                                         + sensors['Geomagnetic Z']
                sensors['Pressure'] = payval(22,3) / 2048
                sensors['Illuminance'] = payval(25,2) / 1.2
                sensors['Magnetic'] = hex(payval(27))
                sensors['Steps'] = payval(28,2)
                sensors['Battery Level'] = payval(30)
                sensors['RSSI'] = dev.rssi

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

            if isRohmMedal == 'Sensor Kit RH':
                sensors['ID'] = hex(payval(2,2))
                sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                sensors['Illuminance'] = payval(6,2) / 1.2
                sensors['SEQ'] = payval(8)
                sensors['Condition Flags'] = bin(int(val[16:18],16))
                sensors['Accelerometer X'] = payval(10,2,True) / 4096
                sensors['Accelerometer Y'] = payval(12,2,True) / 4096
                sensors['Accelerometer Z'] = payval(14,2,True) / 4096
                sensors['Accelerometer'] = (sensors['Accelerometer X'] ** 2\
                                          + sensors['Accelerometer Y'] ** 2\
                                          + sensors['Accelerometer Z'] ** 2) ** 0.5
                sensors['Geomagnetic X'] = payval(16,2,True) / 10
                sensors['Geomagnetic Y'] = payval(18,2,True) / 10
                sensors['Geomagnetic Z'] = payval(20,2,True) / 10
                sensors['Geomagnetic']  = (sensors['Geomagnetic X'] ** 2\
                                         + sensors['Geomagnetic Y'] ** 2\
                                         + sensors['Geomagnetic Z'] ** 2) ** 0.5
                sensors['Pressure'] = payval(22,3) / 2048
                sensors['RSSI'] = dev.rssi

            if isRohmMedal == 'Spresense Rohm IoT':
                sensors['ID'] = hex(payval(2,2))
                sensors['Temperature'] = -45 + 175 * payval(4,2) / 65536
                sensors['Pressure'] = payval(6,3) / 2048
                sensors['SEQ'] = payval(9)
                sensors['RSSI'] = dev.rssi

            if sensors:
                printval(sensors, 'ID', 0, '')
                printval(sensors, 'SEQ', 0, '')
                printval(sensors, 'Temperature', 2, '℃')
                printval(sensors, 'Humidity', 2, '%')
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
                printval(sensors, 'Magnetic', 0, '')
                printval(sensors, 'Steps', 0, '歩')
                printval(sensors, 'Battery Level', 0, '%')
                printval(sensors, 'RSSI', 0, 'dB')
            isRohmMedal = ''
            # センサ値の取得処理の終了
    # 受信後の処理

    # センサ個別値のファイルを保存
    date=datetime.datetime.today()
    if savedata:
        for sensor in sensors:
            if (sensor.find(' ') >= 0 or len(sensor) <= 5 or sensor == 'Magnetic') and sensor != 'Color R':
                continue
            s = date.strftime('%Y/%m/%d %H:%M')
          # s += ', ' + sensor
            s += ', ' + str(round(sensors[sensor],3))
            if sensor == 'Color R':
                s += ', ' + str(round(sensors['Color R'],3))
                s += ', ' + str(round(sensors['Color G'],3))
                s += ', ' + str(round(sensors['Color B'],3))
                s += ', ' + str(round(sensors['Color IR'],3))
                sensor = 'Color'
            if sensor == 'Accelerometer':
                s += ', ' + str(round(sensors['Accelerometer X'],3))
                s += ', ' + str(round(sensors['Accelerometer Y'],3))
                s += ', ' + str(round(sensors['Accelerometer Z'],3))
            if sensor == 'Geomagnetic':
                s += ', ' + str(round(sensors['Geomagnetic X'],3))
                s += ', ' + str(round(sensors['Geomagnetic Y'],3))
                s += ', ' + str(round(sensors['Geomagnetic Z'],3))
          # print(s, '-> ' + sensor + '.csv') 
            save(sensor + '.csv', s)

    # クラウドへの送信処理
    if int(ambient_chid) == 0 or not isMedalAvail or time < ambient_interval / interval:
        time += 1
        continue
    time = 0
    isMedalAvail = False
    body_dict['d1'] = sensors['Temperature']
    body_dict['d2'] = sensors['Humidity']
    body_dict['d3'] = sensors['Pressure']
    body_dict['d4'] = sensors['Illuminance']
    body_dict['d5'] = sensors['Accelerometer']
    body_dict['d6'] = sensors['Geomagnetic']
    body_dict['d7'] = sensors['Steps']
    body_dict['d8'] = sensors['Battery Level']

    print(head_dict)                                # 送信ヘッダhead_dictを表示
    print(body_dict)                                # 送信内容body_dictを表示
    post = urllib.request.Request(url_s, json.dumps(body_dict).encode(), head_dict)
                                                    # POSTリクエストデータを作成
    try:                                            # 例外処理の監視を開始
        res = urllib.request.urlopen(post)          # HTTPアクセスを実行
    except Exception as e:                          # 例外処理発生時
        print(e,url_s)                              # エラー内容と変数url_sを表示
    res_str = res.read().decode()                   # 受信テキストを変数res_strへ
    res.close()                                     # HTTPアクセスの終了
    if len(res_str):                                # 受信テキストがあれば
        print('Response:', res_str)                 # 変数res_strの内容を表示
    else:
        print('Done')                               # Doneを表示


''' 実行結果の一例
pi@raspberrypi:~ $ cd
pi@raspberrypi:~ $ git clone http://github.com/bokunimowakaru/esp
pi@raspberrypi:~ $ cd ~/esp/rohm_sensors
pi@raspberrypi:~/esp/rohm_sensors $ sudo ./ble_logger_sens_scan_rh.py

Device xx:xx:xx:XX:XX:XX (public), RSSI=-56 dB
  Flags = 06
  Complete Local Name = R
  Manufacturer = 01004c6cf10093009aff59ff0a0fc40080fee0fcdf521f
    ID            = 0x1
    SEQ           = 147
    Temperature   = 29.03 ℃
    Pressure      = 1002.359 hPa
    Illuminance   = 200.8 lx
    Accelerometer = 0.941 g ( -0.025 -0.041 0.94 g)
    Geomagnetic   = 90.9 uT ( 19.6 -38.4 -80.0 uT)
    RSSI          = -69 dB
'''
