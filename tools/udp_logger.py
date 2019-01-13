#!/usr/bin/env python
# coding: utf-8
# UDPを受信する
# Copyright (c) 2018 Wataru KUNINO

from __future__ import print_function
import sys
import socket
import datetime

buf_n= 128                                              # 受信バッファ容量(バイト)
argc = len(sys.argv)                                    # 引数の数をargcへ代入
print('UDP Logger (usage: '+sys.argv[0]+' port)')       # タイトル表示
if argc == 2:                                           # 入力パラメータ数の確認
    port = sys.argv[1]                                  # ポート番号を設定
    if port < 1 or port > 65535:                        # ポート1未満or65535超の時
        port = 1024                                     # UDPポート番号を1024に
else:
    port = 1024
print('Listening UDP port', port, '...')                # ポート番号表示
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # ソケットを作成
if sock:                                                # 作成に成功したとき
    sock.bind(('', port))                               # ソケットに接続
    while sock:                                         # 永遠に繰り返す
        udp=sock.recv(buf_n)                            # UDPパケットを取得
        udp=udp.replace('\r','').replace('\n',' ')      # 改行を削除
        date=datetime.datetime.today()                  # 日付を取得
        print(date.strftime('%Y/%m/%d %H:%M'), end='')  # 日付を出力
        print(', '+udp)                                 # 受信データを出力
    sock.close()                                        # ソケットの切断
