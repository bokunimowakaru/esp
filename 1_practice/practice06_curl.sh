#!/bin/bash
hr="----------------------------------------------------------------------------"
echo $hr                                                    # 水平線を表示

# NICTによるネットワークを利用した時刻配信サービスは終了しました。
# https://jjy.nict.go.jp/httphttps-index.html
#
# 技術情報
# https://jjy.nict.go.jp/QandA/reference/http-archive.html
#echo -n "現在の時刻 (NICT): "                              # テキスト表示
#curl -s ntp-a1.nict.go.jp/cgi-bin/time                     # 時刻を取得して表示
echo -n "現在の時刻 (bokunimo.net): "                       # テキスト表示
curl -s bokunimo.net/iot/cq/null --head |grep -i "^date:"   # 時刻を取得して表示

echo $hr                                                    # 水平線を表示
#echo -n "天気予報 (Yahoo!): "                              # テキスト表示
#curl -s rss.weather.yahoo.co.jp/rss/days/6200.xml\
#|cut -d'<' -f17|cut -d'>' -f2|tail -1                      # 天気を取得して表示

echo -n "天気予報 (気象庁): "                               # テキスト表示
city_id=130000                              # 気象庁=130000(東京地方など)
                                            # 大阪管区気象台=270000(大阪府など)
                                            # 京都地方気象台=260000(南部など)
                                            # 横浜地方気象台=140000(東部など)
                                            # 銚子地方気象台=120000(北西部など)
                                            # 名古屋地方気象台=230000(西部など)
                                            # 福岡管区気象台=400000(福岡地方など)
curl -s https://www.jma.go.jp/bosai/forecast/data/forecast/{$city_id}.json\
|tr "," "\n"|grep weathers|head -1|cut -d'"' -f4

echo $hr                                                    # 水平線を表示
echo "著者からのメッセージ: "                               # テキスト表示
curl -s http://bokunimo.net/bokunimowakaru/cq/esp2.txt -o tmp_boku.txt~
                                                            # 取得・ファイル保存
grep '<title>' tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # タイトルを抽出
grep '<descr>' tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # メッセージを抽出
grep '<info>'  tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # お知らせを抽出
grep '<state>' tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # 近況を抽出
grep '<url>'   tmp_boku.txt~ |cut -f2|cut -d'<' -f1         # URLを抽出
grep '<date>'  tmp_boku.txt~ |tr '<' ' '|awk '{print $2}'   # 更新日を抽出
\rm tmp_boku.txt~                                           # ファイルの消去
echo $hr                                                    # 水平線を表示

<< 実行例
pi@raspi4metal:~/esp/1_practice $ ./practice06_curl.sh
----------------------------------------------------------------------------
現在の時刻 (bokunimo.net): Date: Sat, 28 Jan 2023 15:05:18 GMT
----------------------------------------------------------------------------
天気予報 (気象庁): くもり　夜遅く　晴れ　所により　夜のはじめ頃　まで　雪か雨
----------------------------------------------------------------------------
著者からのメッセージ:
トランジスタ技術2017年3月号をお買い上げいただきありがとうございました。
3行目にお知らせ、4行目に近況、5行目に関連URL、6行目に更新日が入ります。
Yahoo!ジオシティーズの終了に伴いサポートページを移転しました。
ブックマークなどに登録していただけるよう、お願いいたします。
https://bokunimo.net/bokunimowakaru/cq/esp/
2018/12/25
----------------------------------------------------------------------------
実行例
################################################################################
