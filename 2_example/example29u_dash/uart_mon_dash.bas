new
' IchigoJam Ö³ BASIC ÌßÛ¸Þ×Ñ
1 ?"WiFi ¾Ý» for IchigoJam"
2 M=0:'0:MAC, 3:ÃÞÊÞ²½ Ò²
3 W=1:'WiFiÁ¬ÝÈÙ
100 'init
110 uart 3
120 ?"mode=";M
130 ?"channel=1"
140 ?"phone=1,F4:37:B7:00:00:00
150 ?"phone=2,1C:91:48:00:00:00
160 ?"phone=3,2C:20:0B:00:00:00
170 ?"adash=1,18:74:2E:00:00:00
180 ?"adash=2,B4:7C:9C:00:00:00
190 ?"adash=3,B4:7C:9C:4D:00:00
200 cls
300 'det
310 if inkey() goto 310
320 if inkey()<>asc("'") goto 320
330 if inkey() '
340 I=0:C=0:N=0
500 'get
510 K=inkey()
520 if M=0 ? chr$(K);
530 if I=0 then C=K
540 if I=6 then N=K
550 I=I+1
560 if K<>10 goto 500
570 if M=0 goto 300
800 'print
810 if C=asc("a") ?"adash ";
820 if C=asc("p") ?"phone ";
830 ? chr$(N);" ¦ ¹Ý¼­Â¼Ï¼À
840 goto 300
