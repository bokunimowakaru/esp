#!/bin/bash
hr="----------------------------------------------------------------------------"
echo $hr                                                    # 水平線を表示
echo -n "現在の時刻 (NICT): "                               # テキスト表示
curl -s ntp-a1.nict.go.jp/cgi-bin/time                      # 時刻を取得して表示
echo $hr                                                    # 水平線を表示
echo -n "天気予報 (Yahoo!): "                               # テキスト表示
curl -s rss.weather.yahoo.co.jp/rss/days/6200.xml\
|cut -d'<' -f17|cut -d'>' -f2|tail -1                       # 天気を取得して表示
echo $hr                                                    # 水平線を表示
echo "著者からのメッセージ: "                               # テキスト表示
curl -s www.bokunimo.net/bokunimowakaru/cq/esp2.txt -o tmp_boku.txt~
                                                            # 取得・ファイル保存
grep '<title>' tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # タイトルを抽出
grep '<descr>' tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # メッセージを抽出
grep '<info>'  tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # お知らせを抽出
grep '<state>' tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # 近況を抽出
grep '<url>'   tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # URLを抽出
grep '<date>'  tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # 更新日を抽出
\rm tmp_boku.txt~                                           # ファイルの消去
echo $hr                                                    # 水平線を表示
