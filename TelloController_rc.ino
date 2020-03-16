#define LOAD_FONT2
#define LOAD_FONT4
#include <M5Stack.h>
#include "utility/MPU9250.h"
#include <WiFi.h>
#include <WiFiUdp.h>
// TELLOのSSID
const char* TELLO_SSID = "TELLO-XXXXXX";  // 自分のTelloのWi-Fi SSIDを入力
// TELLOのIP
const char* TELLO_IP = "192.168.10.1";
// TELLO_PORT
const int PORT = 8889;
// UDPまわり
WiFiUDP Udp;
char packetBuffer[255];
String message = "";

MPU9250 IMU;

float x;
float y;
char msgx[6];
char msgy[6];
String status_msg;

// 変更点(1) rcコマンド用文字列変数
char command_str[20];

void setup() {
  //M5Stackの初期設定
  M5.begin();
  //画面表示
  //---タイトル
  M5.Lcd.fillRect(0,0,320,30,TFT_BLUE);
  M5.Lcd.drawCentreString("Tello Controller",160,2,4);
  //---X, Yの表示
  M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
  M5.Lcd.drawCentreString(" Accel-X : ",20,30,2);
  sprintf(msgx,"%-2.2f",x);
  M5.Lcd.drawCentreString(msgx,88,30,2);
  //---X, Y値の表示
  M5.Lcd.drawCentreString(" Accel-Y : ",240,30,2);
  sprintf(msgy,"%-2.2f",y);
  M5.Lcd.drawCentreString(msgy,294,30,2);
  //---ボタンエリア
  M5.Lcd.fillRect(0,217,320,20,TFT_LIGHTGREY);
  //---ボタン文字
  M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
  M5.Lcd.drawCentreString(" TAKE OFF ",64,220,2);
  M5.Lcd.setTextColor(TFT_BLACK,TFT_RED);
  M5.Lcd.drawCentreString(" LANDING  ",250,220,2);
  M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
  M5.Lcd.drawCentreString("CW/CCW_U/D",160,220,2);
  //---方向矢印
  M5.Lcd.fillTriangle(159,40,189,60,129,60,TFT_GREEN);
  M5.Lcd.fillTriangle(159,160,189,140,129,140,TFT_GREEN);
  M5.Lcd.fillTriangle(269,100,220,80,220,120,TFT_GREEN);
  M5.Lcd.fillTriangle(98,80,98,120,49,100,TFT_GREEN);
  //---方向の文字
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.drawCentreString("FORWARD",160,64,2);
  M5.Lcd.drawCentreString("BACK",160,120,2);
  M5.Lcd.drawCentreString("LEFT",120,92,2);
  M5.Lcd.drawCentreString("RIGHT",200,92,2);
  //---メッセージ領域
  M5.Lcd.drawRoundRect(0,180,319,30,4,TFT_WHITE);
  //---メッセージのタイトル文字
  M5.Lcd.setTextColor(TFT_WHITE,TFT_DARKGREEN);
  M5.Lcd.drawCentreString("<Message>",38,170,1);
  //---メッセージの文字
  //M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  //M5.Lcd.drawString(msg,4,190,1);
  //初期設定
  //Wireライブラリを初期化
  Wire.begin();
  //MPU9250を初期化
  IMU.initMPU9250();
  //WiFi通信の開始
  WiFi.begin(TELLO_SSID, "");
  //WiFi接続　接続するまでループ
  while (WiFi.status() != WL_CONNECTED) {
        print_msg("Now, WiFi Connecting......");
        delay(500);
  }
  print_msg("WiFi Connected.");
  // UDP
  Udp.begin(PORT);
  //Telloへ”command”送信
  print_msg("sendMessage commend");
  tello_command_exec("command");  
}

void loop() {
  // put your main code here, to run repeatedly:
  //x,y値の取得と表示
  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01){
    IMU.readAccelData(IMU.accelCount);
    IMU.getAres();
    x = IMU.accelCount[0] * IMU.aRes;
    y = IMU.accelCount[1] * IMU.aRes;
    sprintf(msgx,"%-2.2f",x);
    M5.Lcd.drawCentreString("      ",88,30,2);
    M5.Lcd.drawCentreString(msgx,88,30,2);
    sprintf(msgy,"%-2.2f",y);
    M5.Lcd.drawCentreString("      ",294,30,2);
    M5.Lcd.drawCentreString(msgy,294,30,2);
    print_msg("Operation Start!"); 

    // 変更点(2) ボタンに応じた処理
    //ボタンA処理
    if(M5.BtnA.wasPressed()) {
      //離陸
      print_msg("TAKE OFF"); 
      tello_command_exec("takeoff");
    }

    //ボタンB処理
    if(M5.BtnB.isPressed()) {
      //---方向の文字 Bボタンを押しているとき
      M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
      M5.Lcd.drawCentreString("  DOWN  ",160,64,2);
      M5.Lcd.drawCentreString("  UP  ",160,120,2);
      M5.Lcd.drawCentreString(" CCW",120,92,2);
      M5.Lcd.drawCentreString("  CW ",200,92,2);
    } else {
      //---方向の文字 通常時
      M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
      M5.Lcd.drawCentreString("FORWARD",160,64,2);
      M5.Lcd.drawCentreString("BACK",160,120,2);
      M5.Lcd.drawCentreString("LEFT",120,92,2);
      M5.Lcd.drawCentreString("RIGHT",200,92,2);
    }

    //ボタンC処理    
    if(M5.BtnC.wasPressed()) {
      //着陸
      print_msg("LAND");
      tello_command_exec("land");
    }

    // 変更点(3) 傾きに応じたコマンド送信
    if (fabs(x)> 0.3 || fabs(y)> 0.3){
        // 傾きx,yに応じてrcコマンドの文字列を作成
        
        //ボタンB処理
        if( M5.BtnB.isPressed() ) {
            sprintf(command_str,"rc 0 0 %d %d",int(y*100), int(-x*100) ); // 上昇下降と旋回
        } else {
            sprintf(command_str,"rc %d %d 0 0",int(-x*100), int(-y*100) ); // 左右移動と前後進
        }
        tello_command_exec(command_str);  // rcコマンドを送信
    } else {
        // 傾いていない時は停止命令を送信し続ける
        tello_command_exec("rc 0 0 0 0");
    }
    delay(50);  // 500ミリ秒のウェイトを50ミリ秒に減らした
  }
  M5.update();
}

/////////////////////////////
//      ユーザ関数定義       //
/////////////////////////////

// 画面メッセージエリアへ状況メッセージ表示
void print_msg(String status_msg){
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.drawString("                          ",4,190,1);
  M5.Lcd.drawString(status_msg,4,190,1);
  status_msg="";
}

// Telloへメッセージ送信＆コマンド実行
void tello_command_exec(char* tello_command){
  Udp.beginPacket(TELLO_IP, PORT);
  Udp.printf(tello_command);
  Udp.endPacket();

  // 変更点(3) Telloからの応答を無視
  //message = listenMessage();  // UDP受信の関数を走らせない

  delay(10);
}

// Telloからのメッセージ受信
String listenMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remoteIp = Udp.remoteIP();
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
  }
  delay(10);
  return (char*) packetBuffer;
}
