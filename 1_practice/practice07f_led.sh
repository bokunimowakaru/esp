#!/bin/bash
# 天気をLEDの状態で表示する(フルカラーLED対応版)
#

IP_LED="192.168.0.2"                                        # ワイヤレスLEDのIP

city_id=130000                              # 気象庁=130000(東京地方など)
                                            # 大阪管区気象台=270000(大阪府など)
                                            # 京都地方気象台=260000(南部など)
                                            # 横浜地方気象台=140000(東部など)
                                            # 銚子地方気象台=120000(北西部など)
                                            # 名古屋地方気象台=230000(西部など)
                                            # 福岡管区気象台=400000(福岡地方など)

while true; do                                              # 永久ループ
#WEATHER=`curl -s rss.weather.yahoo.co.jp/rss/days/43.xml\
#|cut -d'<' -f17|cut -d'>' -f2|tail -1\
#|cut -d' ' -f5|cut -c1-3`                                   # 天気を取得する
WEATHER=`curl -s https://www.jma.go.jp/bosai/forecast/data/forecast/{$city_id}.json\
|tr "," "\n"|grep weathers|head -1|cut -d'"' -f4|rev|sed -e "s/\(.*\)　//1"|rev`
echo -n `date "+%Y/%m/%d %R"`", "$WEATHER", "               # テキスト表示
case $WEATHER in                                            # 天気に応じた処理
    # 解説 変数COMへ制御コマンドR,G,Bのいずれかを代入する
    "晴" )  COM="R";;                                       # 晴の時は赤色点灯
    "くもり" )  COM="G";;                                   # 曇の時は緑色点灯
    * ) COM="B";;                                           # その他の青色点灯
esac                                                        # caseの終了

# 解説 3つのLEDを一度消灯する
curl -s -m3 $IP_LED -XPOST -d"L=0" > /dev/null              # LEDを消灯する<追加>

# 解説 制御コマンドを値=1とともに送信する
RES=`curl -s -m3 $IP_LED -XPOST -d"$COM=1"|grep "<p>"|grep -v "http"\
|cut -d'>' -f2|cut -d'<' -f1`                               # LED制御<修正>
if [ -n "$RES" ]; then                                      # 応答があった場合
    echo $RES                                               # 応答内容を表示
else                                                        # 応答が無かった場合
    echo "ERROR"                                            # ERRORを表示
fi                                                          # ifの終了
sleep 1800                                                  # 待ち時間処理(30分)
done                                                        # whileに戻る
