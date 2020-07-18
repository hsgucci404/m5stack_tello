#include <M5Atom.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// TELLO setting
const char* TELLO_SSID = "TELLO-xxxxxx";  // Set your Tello's SSID
const char* TELLO_IP = "192.168.10.1";    // TELLO IP
const int PORT = 8889;                    // TELLO_PORT

// wifi variables
WiFiUDP Udp;
char packetBuffer[255];
String message = "";
char command_str[20];

// MPU6886 variables
bool IMU6886Flag = false;

// acceleration value
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float x, y, z;

// the setup routine runs once when you press reset:
void setup() {
  M5.begin();
  if (M5.IMU.Init() != 0)
    IMU6886Flag = false;
  else
    IMU6886Flag = true;

  // initialize
  Wire.begin();
  WiFi.begin(TELLO_SSID, "");   // default Tello has no password

  // wait WiFi conecttion
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // UDP start
  Udp.begin(PORT);

  // send "command" to Tello, then Tello goes into SDK mode
  tello_command_exec("command");

  delay(500); // delay for stability
}

// the loop routine runs over and over again forever:
void loop() {
  // get acceleration data
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  x = accX;
  y = accY;
  z = accZ;

  // button B processing
  // Landing
  if (M5.Btn.wasPressed()) {  // push Btn B quickly, then Land
    tello_command_exec("land");
    delay(500);
  }
  // Takeoff
  if (M5.Btn.pressedFor(300)) { // press Btn B for 300msec, then Takeoff
    tello_command_exec("takeoff");
    delay(500);
  }

  // z < 0, that means normal posture
  if ( z < 0 ) {
    // create rc command only when tilted greatly
    if (fabs(x) > 0.3 || fabs(y) > 0.3) {
      sprintf(command_str, "rc %d %d 0 0", int(x * 100), int(-y * 100) );   // a:left/right, b:forward/backward
    } else {
      sprintf(command_str, "rc 0 0 0 0" );
    }
    tello_command_exec(command_str);  // send rc command
  } else if ( z > 0 ) {   // z > 0, that means upside down posture
    // create rc command only when tilted greatly
    if (fabs(x) > 0.3 || fabs(y) > 0.3) {
      sprintf(command_str, "rc 0 0 %d %d", int(y * 100), int(-x * 100) );   // c:up/down d:yaw
    } else {
      sprintf(command_str, "rc 0 0 0 0" );
    }
    tello_command_exec(command_str);  // send rc command
  }

  M5.update();
}

// UDP send command
void tello_command_exec(char* tello_command) {
  Udp.beginPacket(TELLO_IP, PORT);
  Udp.printf(tello_command);
  Udp.endPacket();
  delay(100);
}
