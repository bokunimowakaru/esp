/*******************************************************************************
Google カレンダー(予定表) から予定を取得する

                                                Copyright (c) 2018 Wataru KUNINO
********************************************************************************
実行方法
    1. Google Apps Script(https://script.google.com/)へアクセス
    2. 本スクリプトをペーストし、画面内メニューの[ファイル]→[保存]を実行
    3. [プロジェクト名の編集]画面でプロジェクト名を入力して、[OK]をクリック
    4. [公開]メニューから[ウェブアプリケーションとして導入]を選択
    5. [承認が必要です]のメッセージが表示されるので[許可を確認]をクリック
    6. [アカウント選択]画面で自分のアカウントを選択し、[許可]をクリック
    7. 項目[次のユーザとしてアプリケーションを実行]で自分のアカウントを選択
    8. 項目[アプリケーションにアクセスできるユーザー]で[全員]を選択
    9. [導入]を押すと公開URLが表示されるので、コピーする
    10. コピーした公開URLをブラウザでアクセスし、動作確認

出力形式
    Length,予定数
    時,分,タイトル1
    時,分,タイトル2
    ...(6時間以内の全予定)...
    [EOF]

参考文献：
    https://github.com/bokunimowakaru/esp/tree/master/2_example/example63_google
    https://github.com/wilda17/ESP8266-Google-Calendar-Arduino
    https://developers.google.com/apps-script/reference/calendar/calendar-app
*******************************************************************************/

function doGet(){
  return ContentService.createTextOutput(GetEvents());
}

function GetEvents(){
  var Now = new Date();
  var EndTime = new Date(Now.getTime() + (6 * 60 * 60 * 1000));
  Logger.log("Start " + Now);
  Logger.log("End " + EndTime);
  var events = CalendarApp.getEvents(Now, EndTime);
  Logger.log(events.length + " Events");
  str = "Length," + events.length + '\n';
  for (var i = 0; i < events.length; i++){
    str += events[i].getStartTime().getHours() + ',';
    str += events[i].getStartTime().getMinutes() + ',';
    str += events[i].getTitle();
    str += '\n';
  }
  str += "[EOF]";
  Logger.log("retrun str\n" + str);
  return str;
}
