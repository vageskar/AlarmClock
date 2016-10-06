// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <Adafruit_LEDBackpack.h>
#include "Adafruit_GFX.h"
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"


#define TIME_24_HOUR  true
#define DISPLAY_ADDRESS 0x70
#define SFX_TX 12
#define SFX_RX 13
#define SFX_RST 14


RTC_DS3231 rtc;
Adafruit_7segment clockDisplay = Adafruit_7segment();
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

int hours = 0;
int minutes = 0;
int seconds = 0;
int days = 0;
int nextDay = 0;
int alarm1H = 0;
int alarm1M = 0;
int alarm2H = 0;
int alarm2M = 0;
int alarmS = 0;
int lastMin = 0;
int btnPin = A0;

unsigned long readTime = 0;

boolean alarm1 = false;
boolean alarm2 = false;
boolean alarm1Days[] = {false, false, false, false, false, false, false};
boolean alarm2Days[] = {false, false, false, false, false, false, false};

byte brightness = 12;

const char* ssid = "Inteno-78FC";
const char* password = "JNC9KIZZPR64";

const char* host = "192.168.1.168";
const int port = 2609;
WiFiClient conn;


void setup () {

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  pinMode(btnPin, INPUT);
  Serial.begin(115200);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  WiFi.begin(ssid, password);
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.setBrightness(brightness); //0-15
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  ss.begin(9600);
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
}

void loop () {
  DateTime dt = rtc.now();
  hours = dt.hour();
  minutes = dt.minute();
  seconds = dt.second();
  days = dt.dayOfTheWeek();
  nextDay = days == 6 ? 0 : days + 1;

  int displayValue = hours * 100 + minutes;
  clockDisplay.print(displayValue, DEC);
  if (hours == 0) {
    clockDisplay.writeDigitNum(1, 0);
    if (minutes < 10) {
      clockDisplay.writeDigitNum(3, 0);
    }
  }
  clockDisplay.drawColon(true);
  if(alarm1Days[nextDay] || alarm2Days[nextDay]){
    clockDisplay.writeDigitRaw(2,0x10);
  }
  //clockDisplay.writeDigitRaw(2,0x08);
  clockDisplay.writeDisplay();
  if (alarm1H == hours && alarm1M == minutes) {
    sfx.playTrack(1);
  }
   if (lastMin != minutes) {
    getAlarms();
  }
  lastMin = minutes;
}

void getAlarms(){
  if (!conn.connected()) {
    if (!conn.connect(host, port)) {
      Serial.println("Not connected");
      return;
    }
  }
  conn.println("Get all");
  delay(100);
  while(conn.available() > 0){
    String line = conn.readStringUntil('\n');
    String alarm1Line = line.substring(0,25);
    String alarm2Line = line.substring(27,52);
    
    String a1H = alarm1Line.substring(7,9);
    String a1M = alarm1Line.substring(10,12);
    String a1Days = alarm1Line.substring(13,25);
    
    String a2H = alarm2Line.substring(7,9);
    String a2M = alarm2Line.substring(10,12);
    String a2Days = alarm2Line.substring(13,25);
    
    alarm1H = a1H.toInt();
    alarm1M = a1M.toInt();
    alarm2H = a2H.toInt();
    alarm2M = a2M.toInt();
    Serial.print("A1:");
    Serial.print(a1H);
    Serial.println(a1M);
    Serial.print("A2:");
    Serial.print(a2H);
    Serial.println(a2M);
    Serial.println();
  }
  conn.stop();
}

void setAlarmDays(String aDays, boolean b[]){
  int str_len = aDays.length() + 1;
  char values[str_len];
  aDays.toCharArray(values, str_len); 
  int pos = 0;
  for(int x = 0; x < str_len; x += 2, pos++){
    if(values[x] == '1'){
      b[pos] = true;
    }
    else if(values[x] == '0'){
      b[pos] = false;
    }
  }
}

