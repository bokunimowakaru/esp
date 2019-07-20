#!/usr/bin/env python3
# coding: utf-8
# UDPを受信する
# Copyright (c) 2018-2019 Wataru KUNINO

import sys
import socket
import datetime

buf_n= 128                                              # 受信バッファ容量(バイト)
argc = len(sys.argv)                                    # 引数の数をargcへ代入
print('UDP Logger (usage: '+sys.argv[0]+' port)')       # タイトル表示
if argc >= 2:                                           # 入力パラメータ数の確認
    port = sys.argv[1]                                  # ポート番号を設定
    if port < 1 or port > 65535:                        # ポート1未満or65535超の時
        port = 1024                                     # UDPポート番号を1024に
else:
    port = 1024
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
    udp = udp.decode()                                  # UDPデータを文字列に変換
    s=''                                                # 表示用の文字列変数s
    for c in udp:                                       # UDPパケット内
        if ord(c) >= ord(' ') and ord(c) <= ord('~'):   # 表示可能文字
            s += c                                      # 文字列sへ追加
    date=datetime.datetime.today()                      # 日付を取得
    print(date.strftime('%Y/%m/%d %H:%M')+', ', end='') # 日付を出力
    print(udp_from[0], end='')                          # 送信元アドレスを出力
    print(', ' + s, flush=True)                         # 受信データを出力
sock.close()                                            # ソケットの切断
