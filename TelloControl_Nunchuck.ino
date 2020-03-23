#define LOAD_FONT2
#define LOAD_FONT4
#include <M5Stack.h>
#include <WiiChuck.h>
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


float ail,ele,thr,rud;
char msg_jx[6];
char msg_jy[6];
char msg_ax[6];
char msg_ay[6];
String status_msg;
char command_str[20];

Accessory nunchuck1;

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
  sprintf(msg_jx,"%-2.2f",ail);
  M5.Lcd.drawCentreString(msg_jx,88,30,2);
  //---X, Y値の表示
  M5.Lcd.drawCentreString(" Joy-Y : ",240,30,2);
  sprintf(msg_jy,"%-2.2f",ele);
  M5.Lcd.drawCentreString(msg_jy,294,30,2);

  M5.Lcd.drawCentreString(" Accel-X : ",20,50,2);
  sprintf(msg_ax,"%-2.2f",rud);
  M5.Lcd.drawCentreString(msg_ax,88,50,2);
  //---X, Y値の表示
  M5.Lcd.drawCentreString(" Accel-Y : ",240,50,2);
  sprintf(msg_ay,"%-2.2f",thr);
  M5.Lcd.drawCentreString(msg_ay,294,50,2);
  
  
  //---ボタンエリア
  M5.Lcd.fillRect(0,217,320,20,TFT_LIGHTGREY);
  //---ボタン文字
  M5.Lcd.setTextColor(TFT_BLACK,TFT_YELLOW);
  M5.Lcd.drawCentreString(" TAKE OFF ",64,220,2);
  M5.Lcd.setTextColor(TFT_BLACK,TFT_RED);
  M5.Lcd.drawCentreString(" LANDING  ",250,220,2);
  M5.Lcd.setTextColor(TFT_BLACK,TFT_CYAN);
  M5.Lcd.drawCentreString(" STOP ",160,220,2);
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

  //Wiiヌンチャクを初期化
  nunchuck1.begin();
  if (nunchuck1.type == Unknown) {
    nunchuck1.type = NUNCHUCK;
  }
    
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

  delay(500);
  print_msg("Operation Start!"); 
}

void loop() {
  // ヌンチャク値の取得
  nunchuck1.readData();

  ail = (nunchuck1.getJoyX()-128) / 100.0;
  ele = (nunchuck1.getJoyY()-128) / 100.0;
  thr = (nunchuck1.getAccelY()-512) / 256.0;
  rud = (nunchuck1.getAccelX()-512) / 256.0;

  // 画面表示
  sprintf(msg_jx,"%-2.2f",ail);
  M5.Lcd.drawCentreString("      ",88,30,2);
  M5.Lcd.drawCentreString(msg_jx,88,30,2);
  sprintf(msg_jy,"%-2.2f",ele);
  M5.Lcd.drawCentreString("      ",294,30,2);
  M5.Lcd.drawCentreString(msg_jy,294,30,2);

  sprintf(msg_ax,"%-2.2f",rud);
  M5.Lcd.drawCentreString("      ",88,50,2);
  M5.Lcd.drawCentreString(msg_ax,88,50,2);
  sprintf(msg_ay,"%-2.2f",thr);
  M5.Lcd.drawCentreString("      ",294,50,2);
  M5.Lcd.drawCentreString(msg_ay,294,50,2);

  //離陸
  if(M5.BtnA.wasPressed()) {  // ボタンA
    print_msg("TAKE OFF"); 
    tello_command_exec("takeoff");
  }
  if(nunchuck1.getButtonC() ) {   // ヌンチャクCボタン
    print_msg("TAKE OFF"); 
    tello_command_exec("takeoff");
  }

  //停止
  if(M5.BtnB.wasPressed()) {    //ボタンB処理
      print_msg("STOP");  // 通信不良などの暴走時にはボタンBで止める
      tello_command_exec("rc 0 0 0 0");
  }

  //着陸
  if(M5.BtnC.wasPressed()) {    //ボタンC処理    
    print_msg("LAND");
    tello_command_exec("land");
  }
  if(nunchuck1.getButtonZ() ) {   // ヌンチャクZボタン
    print_msg("LAND");
    tello_command_exec("land");
  }

  //スティックの0.3，加速度の0.5は実測値から閾値を設定した
  if (fabs(ail)> 0.3 || fabs(ele)> 0.3 || fabs(thr)> 0.5 || fabs(rud)> 0.5){
      // 不感帯設定　(微弱な加速度値で上昇や旋回しないように)
      ail = (fabs(ail)>0.3)? ail : 0.0;
      ele = (fabs(ele)>0.3)? ele : 0.0;
      thr = (fabs(thr)>0.5)? thr : 0.0;
      rud = (fabs(rud)>0.5)? rud : 0.0;

      sprintf(command_str,"rc %d %d %d %d",int(ail*100), int(ele*100), int(-thr*100), int(rud*100) );
      tello_command_exec(command_str);
  } else {
      tello_command_exec("rc 0 0 0 0");
  }    
  
  delay(50);
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
//  message = listenMessage();
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
