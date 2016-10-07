#!/bin/bash
# 天気をLEDの状態で表示する

DEV_LED="192.168.0.2"                                       # ワイヤレスLEDのIP
while true; do                                              # 永久ループ
WEATHER=`curl -s rss.weather.yahoo.co.jp/rss/days/43.xml\
|cut -d'<' -f17|cut -d'>' -f2|tail -1\
|cut -d' ' -f5|cut -c1-3`                                   # 天気を取得する
echo -n `date "+%Y/%m/%d %R"`", "$WEATHER", "               # テキスト表示
case $WEATHER in                                            # 天気に応じた処理
    "晴" )  LED=1;;                                         # 晴の時は明るく点灯
    "曇" )  LED=-1;;                                        # 曇の時は暗く点灯
    * ) LED=5;;                                             # その他の時は点滅
esac                                                        # caseの終了
RES=`curl -s -m3 $DEV_LED -XPOST -d"L=$LED"|grep "<p>"|grep -v "http"\
|cut -d'>' -f2|cut -d'<' -f1`                               # ワイヤレスLED制御
if [ -n "$RES" ]; then                                      # 応答があった場合
    echo $RES                                               # 応答内容を表示
else                                                        # 応答が無かった場合
    echo "ERROR"                                            # ERRORを表示
fi                                                          # ifの終了
sleep 1800                                                  # 待ち時間処理(30分)
done                                                        # whileに戻る
