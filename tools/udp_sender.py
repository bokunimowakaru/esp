#!/usr/bin/env python
# coding: utf-8
# UDPを送信する
# Copyright (c) 2018 Wataru KUNINO

from __future__ import print_function
import sys
import socket

argc = len(sys.argv)                                    # 引数の数をargcへ代入
print('UDP Sender (usage: '+sys.argv[0]+' port < data)')# タイトル表示
if argc == 2:                                           # 入力パラメータ数の確認
    port = sys.argv[1]                                  # ポート番号を設定
    if port < 1 or port > 65535:                        # ポート1未満or65535超の時
        port = 1024                                     # UDPポート番号を1024に
else:
    port = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # ソケットを作成
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST,1)
if sock:                                                # 作成に成功したとき
    for line in sys.stdin:                              # 標準入力から変数lineへ
        udp=line.replace('\r','').replace('\n','')      # 改行を削除
        print('send : ' + udp)                          # 受信データを出力
        sock.sendto(line+'\n',('255.255.255.255',port)) # UDPブロードキャスト送信
    sock.close()                                        # ソケットの切断
