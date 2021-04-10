#!/usr/bin/env python3
# coding: utf-8
# UDPを受信し、保存する
# 各種IoTセンサ用・測定結果表示プログラム【保存機能つき】【デバイス限定式】
#
# Copyright (c) 2018-2021 Wataru KUNINO

# 初期設定
UDP_PORT = 1024             # UDP待ち受けポート番号(デフォルトは1024)
DEV_CHECK = False           # 未登録デバイス保存(True:破棄,False:UNKNOWNで保存)

# センサ機器用登録デバイス（UDPペイロードの先頭5文字）
sensors = [\
    'temp0','hall0','adcnv','btn_s','pir_s','illum',\
    'temp.','humid','press','envir','accem','rd_sw',\
    'press','e_co2',\
    'actap','awsin','count','esp32','ident','medal',\
    'meter','ocean','river','tftpc','timer','voice',\
    'xb_ac','xb_ct','xb_ev','xb_sw','xbbat','xbbel',\
    'xbgas','xbhum','xblcd','xbled','xbprs','xbrad',\
    'xbsen'\
]

# センサ機器用CSV形式データの項目（数値データ）
csvs = {\
    'pir_s':[('Wake up Switch',''),('PIR Switch','')],\
    'rd_sw':[('Wake up Switch',''),('Reed Switch','')],\
    'temp0':[('Temperature','deg C')],\
    'temp.':[('Temperature','deg C')],\
    'ocean':[('Temperature','deg C'),('RSSI','dBm')],\
    'humid':[('Temperature','deg C'),('Humidity','%')],\
    'press':[('Temperature','deg C'),('Pressure','hPa')],\
    'envir':[('Temperature','deg C'),('Humidity','%'),('Pressure','hPa')],\
    'e_co2':[('Temperature','deg C'),('Humidity','%'),('Pressure','hPa'),('CO2','ppm'),('TVOC','ppb'),('Counter','')],\
    'accem':[('Accelerometer X','g'),('Accelerometer Y','g'),('Accelerometer Z','g')],\
    'actap':[('Power','W'),('Cumulative','Wh'),('Time','Seconds')],\
    'meter':[('Power','W'),('Cumulative','Wh'),('Time','Seconds')],\
    'awsin':[('Participants',''),('Cumulative','')],\
    'xb_ac':[('Usage Time','h'),('Consumption','kWh'),('Prev. Usage Time','h'),('Consumption','kWh')],\
    'xb_ct':[('Power','W')],\
    'xb_ev':[('Illuminance','lx'),('Temperature','deg C'),('Humidity','%')],\
    'xb_sw':[('Reed Switch','')],\
    'xbbel':[('Ringing','')],\
    'xbgas':[('CO','ppm'),('CH4','ppm')],\
    'xbhum':[('Illuminance','lx'),('Temperature','deg C'),('Humidity','%')],\
    'xblcd':[('Illuminance','lx'),('Temperature','deg C')],\
    'xbled':[('Illuminance','lx'),('Temperature','deg C')],\
    'xbprs':[('Temperature','deg C'),('Pressure','hPa')],\
    'xbrad':[('Radiation','uSievert'),('Temperature','deg C'),('Voltage','V')],\
    'xbsen':[('Illuminance','lx'),('Temperature','deg C'),('Low Battery','')]\
}

# センサ機器以外（文字データ入り）の登録デバイス
notifyers = [\
    'adash','atalk','cam_a','ir_in','janke','sound',\
    'xb_ir','xbidt'\
]

# 特定文字列
pingpongs = [
    'Ping','Pong','Emergency','Reset'\
]

devices = list()

import os
import sys
import socket
import datetime

def get_dev_name(s):                                    # デバイス名を取得
    if s.strip() in pingpongs:                          # Ping または Pong
        return s.strip()
    if not s[0:8].isprintable():
        return None                                     # Noneを応答
    if s[5] == '_' and s[7] == ',':                     # 形式が一致する時
        if s[0:5] in sensors:                           # センサリストの照合
            return s[0:7]                               # デバイス名を応答
        if s[0:5] in notifyers:                         # センサリストの照合
            return s[0:7]                               # デバイス名を応答
    return None                                         # Noneを応答

def get_val(s):                                         # データを数値に変換
    s = s.replace(' ','')                               # 空白文字を削除
    if s.replace('.','').replace('-','').isnumeric():   # 文字列が数値を示す
        val = float(s)                                  # 小数値に変換
        if float(int(val)) == val:                      # valが整数のとき
            return int(val)                             # 整数値を応答
        else:
            return val                                  # 小数値を応答
    return None                                         # Noneを応答

def save(filename, data):
    try:
        fp = open(filename, mode='a')                   # 書込用ファイルを開く
    except Exception as e:                              # 例外処理発生時
        print(e)                                        # エラー内容を表示
    fp.write(data + '\n')                               # dataをファイルへ
    fp.close()                                          # ファイルを閉じる

buf_n= 128                                              # 受信バッファ容量(バイト)
argc = len(sys.argv)                                    # 引数の数をargcへ代入
print('UDP Logger (usage: '+sys.argv[0]+' port)')       # タイトル表示
if argc >= 2:                                           # 入力パラメータ数の確認
    port = int(sys.argv[1])                             # ポート番号を設定
    if port < 1 or port > 65535:                        # ポート1未満or65535超の時
        port = UDP_PORT                                 # UDPポート番号を1024に
else:
    port = UDP_PORT
print('Listening UDP port', port, '...')                # ポート番号表示
try:
    sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)# ソケットを作成
    sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)    # オプション
    sock.bind(('', port))                               # ソケットに接続
except Exception as e:                                  # 例外処理発生時
    print(e)                                            # エラー内容を表示
    exit()                                              # プログラムの終了

while sock:                                             # 永遠に繰り返す
    udp, udp_from = sock.recvfrom(buf_n)                # UDPパケットを取得
    try:
        udp = udp.decode()                              # UDPデータを文字列に変換
    except Exception as e:                              # 例外処理発生時
        print(e)                                        # エラー内容を表示
        continue                                        # whileの先頭に戻る
    if len(udp) <= 4:
        continue
    dev = get_dev_name(udp)                             # デバイス名を取得
    if dev is None:                                     # 不適合
        if DEV_CHECK:                                   # デバイス選別モード時
            continue                                    # whileに戻る
        dev = 'UNKNOWN'                                 # 不明デバイス

    vals = list()
    if len(udp) > 8:
        vals = udp[8:].strip().split(',')               # 「,」で分割
    date = datetime.datetime.today()                    # 日付を取得
    date = date.strftime('%Y/%m/%d %H:%M')              # 日付を文字列に変更
    s = ''                                              # 文字列変数
    if dev[0:5] in sensors:
        for val in vals:                                # データ回数の繰り返し
            i = get_val(val)                            # データを取得
            s += ', '                                   # 「,」を追加
            if i is not None:                           # データがある時
                s += str(i)                             # データを変数sに追加
    else:
        s = ', '                                        # 文字列変数
        for c in udp:                                   # UDPパケット内
            if ord(c) >= ord(' ') and ord(c) <= ord('~'):   # 表示可能文字
                s += c                                  # 文字列sへ追加
    filename = 'log_' + dev + '.csv'                    # ファイル名を作成
    if dev not in devices:
        print('NEW Device,',dev)
        devices.append(dev)
        if not os.path.exists(filename):
            fp = open(filename, mode='w')               # 書込用ファイルを開く
            fp.write('YYYY/MM/dd hh:mm, IP Address')    # CSV様式
            column = csvs.get(dev[0:5])
            if column is not None:
                for col in column:
                    if col[1] == '':
                        fp.write(', ' + col[0])
                    else:
                        fp.write(', ' + col[0] + '(' + col[1] + ')')
            fp.write('\n')
            fp.close()                                  # ファイルを閉じる
    print(date + ', ' + dev + ', ' + udp_from[0], end = '')  # 日付,送信元を表示
    print(s, '-> ' + filename, flush=True)              # 受信データを表示
    save(filename, date + ', ' + udp_from[0] + s)       # ファイルに保存
sock.close()                                            # ソケットの切断
