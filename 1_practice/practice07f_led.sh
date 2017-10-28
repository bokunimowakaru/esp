#!/bin/bash
# 天気をLEDの状態で表示する(フルカラーLED対応版)
#
# http://rss.weather.yahoo.co.jp/rss/days/7.xml		福島県（福島市）
# http://rss.weather.yahoo.co.jp/rss/days/13.xml	東京都（東京）
# http://rss.weather.yahoo.co.jp/rss/days/23.xml	愛知県（西部・名古屋）
# http://rss.weather.yahoo.co.jp/rss/days/26.xml	京都府（南部・京都市）
# http://rss.weather.yahoo.co.jp/rss/days/27.xml	大阪府（大阪市）
# http://rss.weather.yahoo.co.jp/rss/days/28.xml	兵庫県（神戸市）
# http://rss.weather.yahoo.co.jp/rss/days/43.xml	熊本県（熊本市）
#
# ご注意：
#   ・Yahoo!天気・災害の情報を商用で利用する場合はYahoo! Japanの承諾が必要です。
#   ・Yahoo!サービスの利用規約にしたがって利用ください。
#           https://about.yahoo.co.jp/docs/info/terms/

IP_LED="192.168.0.2"                                        # ワイヤレスLEDのIP
while true; do                                              # 永久ループ
WEATHER=`curl -s rss.weather.yahoo.co.jp/rss/days/43.xml\
|cut -d'<' -f17|cut -d'>' -f2|tail -1\
|cut -d' ' -f5|cut -c1-3`                                   # 天気を取得する
echo -n `date "+%Y/%m/%d %R"`", "$WEATHER", "               # テキスト表示
case $WEATHER in                                            # 天気に応じた処理
    # 解説 変数COMへ制御コマンドR,G,Bのいずれかを代入する
    "晴" )  COM="R";;                                       # 晴の時は赤色点灯
    "曇" )  COM="G";;                                       # 曇の時は緑色点灯
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
