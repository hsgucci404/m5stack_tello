#define LOAD_FONT2
#define LOAD_FONT4
#include <M5Stack.h>
//#include "utility/MPU9250.h"  // ジャイロ・加速度センサは使わない
#include "Wire.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#define FACE_JOY_ADDR 0x5e

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

float x;
float y;
char msgx[6];
char msgy[6];
String status_msg;

// rcコマンド用文字列変数
char command_str[20];

// FACE Joystickの初期化
void JoyInit(){
  Wire.begin();
  for (int i = 0; i < 256; i++)
  {
    Wire.beginTransmission(FACE_JOY_ADDR);
    Wire.write(i % 4);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.write(random(256) * (256 - i) / 256);
    Wire.endTransmission();
    delay(2);
  }
  Led(0, 0, 0, 0);
  Led(1, 0, 0, 0);
  Led(2, 0, 0, 0);
  Led(3, 0, 0, 0);
}

// Joystick点灯制御
void Led(int indexOfLED, int r, int g, int b){
  Wire.beginTransmission(FACE_JOY_ADDR);
  Wire.write(indexOfLED);
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}


// 初期化関数
void setup() {
  //M5Stackの初期設定
  M5.begin();
  //画面表示
  //---タイトル
  M5.Lcd.fillRect(0,0,320,30,TFT_BLUE);
  M5.Lcd.drawCentreString("Tello Controller",160,2,4);
  //---X, Yの表示
  M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
  M5.Lcd.drawCentreString(" Joy-X : ",20,30,2);
  sprintf(msgx,"%-2.2f",x);
  M5.Lcd.drawCentreString(msgx,88,30,2);
  //---X, Y値の表示
  M5.Lcd.drawCentreString(" Joy-Y : ",240,30,2);
  sprintf(msgy,"%-2.2f",y);
  M5.Lcd.drawCentreString(msgy,294,30,2);
  //---ボタンエリア
  M5.Lcd.fillRect(0,217,320,20,TFT_LIGHTGREY);
  //---ボタン文字
  M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
  M5.Lcd.drawCentreString(" TAKE OFF ",64,220,2);
  M5.Lcd.setTextColor(TFT_BLACK,TFT_RED);
  M5.Lcd.drawCentreString(" LANDING  ",250,220,2);
//  M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
//  M5.Lcd.drawCentreString("CW/CCW_U/D",160,220,2);
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

  //初期設定
  JoyInit();

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

// FACE Joystick用変数
uint8_t x_data_L;
uint8_t x_data_H;
uint16_t x_data;
uint8_t y_data_L;
uint8_t y_data_H;
uint16_t y_data;
uint8_t button_data;
char data[100];

// ループ関数
void loop() {
  // Joystick値の取得
  Wire.requestFrom(FACE_JOY_ADDR, 5);
  if (Wire.available()) {

    y_data_L = Wire.read();
    y_data_H = Wire.read();
    x_data_L = Wire.read();
    x_data_H = Wire.read();
    
    // Zボタン(0:押している時 1:離している時 )
    button_data = Wire.read();
    
    x_data = x_data_H << 8 |x_data_L;
    y_data = y_data_H << 8 |y_data_L;
  }

  // 0〜1024を±1.0に正規化→本家プログラムと同じ形式に
  x = float(x_data-512) / 512.0;
  y = float(y_data-512) / 512.0;

  sprintf(msgx,"%-2.2f",x);
  M5.Lcd.drawCentreString("      ",88,30,2);
  M5.Lcd.drawCentreString(msgx,88,30,2);
  sprintf(msgy,"%-2.2f",y);
  M5.Lcd.drawCentreString("      ",294,30,2);
  M5.Lcd.drawCentreString(msgy,294,30,2);
  print_msg("Operation Start!"); 

  //ボタンに応じた処理
  // ボタンA処理
  if(M5.BtnA.wasPressed()) {
    //離陸
    print_msg("TAKE OFF"); 
    tello_command_exec("takeoff");
  }

  // ボタンC処理    
  if(M5.BtnC.wasPressed()) {
    //着陸
    print_msg("LAND");
    tello_command_exec("land");
  }

  // アナログスティック押し込みボタン処理
  if( button_data == 0 ) {
    //---Zボタンを押しているとき 上昇下降/左右旋回
    M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    M5.Lcd.drawCentreString("  DOWN  ",160,64,2);
    M5.Lcd.drawCentreString("  UP  ",160,120,2);
    M5.Lcd.drawCentreString(" CCW",120,92,2);
    M5.Lcd.drawCentreString("  CW ",200,92,2);
  } else {
    //---押していないとき 通常表示
    M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    M5.Lcd.drawCentreString("FORWARD",160,64,2);
    M5.Lcd.drawCentreString("BACK",160,120,2);
    M5.Lcd.drawCentreString("LEFT",120,92,2);
    M5.Lcd.drawCentreString("RIGHT",200,92,2);
  }

  // Joystick入力に応じたコマンド送信
  if (fabs(x)> 0.3 || fabs(y)> 0.3){
      // 傾きx,yに応じてrcコマンドの文字列を作成
      
      //ボタンZ処理
      if( button_data == 0 ) {
          sprintf(command_str,"rc 0 0 %d %d",int(-y*100), int(x*100) ); // 上昇下降と旋回
      } else {
          sprintf(command_str,"rc %d %d 0 0",int(x*100), int(y*100) ); // 左右移動と前後進
      }
      tello_command_exec(command_str);  // rcコマンドを送信
  } else {
      // 傾いていない時は停止命令を送信し続ける
      tello_command_exec("rc 0 0 0 0");
  }


  if (x_data > 600){
    Led(2,  0, (button_data)?0:50, (button_data)?50:0);
    Led(0, 0, 0, 0);
  }
  else  if (x_data < 400)
  {
    Led(0,  0, (button_data)?0:50, (button_data)?50:0);
    Led(2, 0, 0, 0);
  } 
  else{
    Led(0,  0, 0,0);
    Led(2, 0, 0, 0);
  }

  
  if (y_data > 600)
  {
    Led(3,  0, (button_data)?0:50, (button_data)?50:0);
    Led(1, 0, 0, 0);
  }
  else if (y_data < 400)
  {
    Led(1,  0, (button_data)?0:50, (button_data)?50:0);
    Led(3, 0, 0, 0);
  }
  else{
    Led(1,  0, 0, 0);
    Led(3, 0, 0, 0);
  }
    
  delay(50);  // 500ミリ秒のウェイトを50ミリ秒に減らした
  
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
