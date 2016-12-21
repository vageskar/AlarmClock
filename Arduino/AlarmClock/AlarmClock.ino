// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <Adafruit_LEDBackpack.h>
#include "Adafruit_GFX.h"
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23008.h"
#include <Time.h>
#include <TimeAlarms.h>


#define TIME_24_HOUR  true
#define DISPLAY_ADDRESS 0x70
#define SFX_TX 12
#define SFX_RX 13
#define SFX_RST 14

#define ALARM_OFF 0
#define SNOOZE 1
#define TIME 2
#define ALARM1 3
#define ALARM2 4
#define SWTIME 5

#define BUTTONS 6


RTC_DS3231 rtc;
Adafruit_MCP23008 mcp;
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
int snoozeH = 0;
int snoozeM = 0;
int lastMin = 0;
int btnPin = A0;

unsigned long readTime = 0;

boolean alarm1On = false;
boolean alarm2On = false;
boolean snooze = false;
boolean snoozeAlarm = false;
boolean alarm1Days[] = {false, false, false, false, false, false, false};
boolean alarm2Days[] = {false, false, false, false, false, false, false};

boolean btn[] = {false, false, false, false, false, false};
boolean lbtn[] = {false, false, false, false, false, false};

byte brightness = 12;

const char* ssid = "Inteno-78FC";
const char* password = "JNC9KIZZPR64";

const char* host = "192.168.1.168";
const int port = 2609;

String inLine = "";
String daysInWeek[] = {"Sunday", "Monday", "Tuesday", "Wedneday", "Thursday", "Friday", "Saturday"};
boolean readData = false;

WiFiClient conn;
WiFiServer server(port);
WiFiServer http(80);

AlarmID_t alarm1;
AlarmID_t alarm2;
timeDayOfWeek_t dow[] = {dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, dowSaturday};

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
  mcp.begin();
  for (int x = 0; x < BUTTONS; x++) {
    mcp.pinMode(x, INPUT);
    mcp.pullUp(x, HIGH);  // turn on a 100K pullup internally
  }
  DateTime dt = rtc.now();
  setTime(dt.unixtime());
  WiFi.begin(ssid, password);
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.setBrightness(brightness); //0-15
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());
  ss.begin(9600);
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  server.begin();
  http.begin();
  setAlarms();
  Serial.println("Start");
}

void loop () {
  DateTime dt = rtc.now();
  readBtn();
  hours = dt.hour();
  if (!btn[SWTIME]) {
    hours--;
  }
  minutes = dt.minute();
  seconds = dt.second();
  days = dt.dayOfTheWeek();
  nextDay = days == 6 ? 0 : days + 1;
  setDisplayShow();
  clockDisplay.writeDisplay();
  deactivateAlarm();
  activateSnooze();
  getAlarms();
  getHttpReq();
  lastMin = minutes;
  updateBtn();
}

void getAlarms() {
  if (!conn.connected()) {
    conn = server.available();
  }
  else if (conn.connected()) {
    if (conn.available() > 0) {
      char c = conn.read();
      Serial.print(c);
      if (c == '$') {
        readData = true;
      }
      else if (c == '%') {
        readData = false;
        Serial.println();
        Serial.println(inLine);
        pharseData(inLine);
        inLine = "";
      }
      else if (readData) {
        inLine += c;
      }
    }
  }
}

void deactivateAlarm() {
  if (!btn[ALARM_OFF] && lbtn[ALARM_OFF]) {
    if ((alarm1On || alarm2On) && isPlaying()) {
      sfx.stop();
      alarm1On = alarm1On xor alarm1On;
      alarm2On = alarm2On xor alarm2On;
    }
  }
}

void playAlarm() {
  if (!isPlaying()) {
    sfx.playTrack(1);
  }
}

void activateSnooze() {
  if (!btn[SNOOZE] && lbtn[SNOOZE] && isPlaying()) {
    sfx.stop();
    if (alarm1On) {
      alarm1 = Alarm.alarmOnce(600, setOffAlarm);
    }
    if (alarm2On) {
      alarm1 = Alarm.alarmOnce(600, setOffAlarm);
    }
  }
}

bool isPlaying() {
  uint32_t current, total;
  if (! sfx.trackTime(&current, &total) ) {
    return false;
  }
  else {
    return true;
  }
}


void setDisplayShow() {
  if (!btn[TIME]) {
    int displayValue = hours * 100 + minutes;
    clockDisplay.print(displayValue, DEC);
    if (hours == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (minutes < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    if (alarm1Days[nextDay] || alarm2Days[nextDay]) {
      clockDisplay.writeDigitRaw(2, 0x12);
    }
    else {
      clockDisplay.writeDigitRaw(2, 0x02);
    }
  }
  else if (!btn[ALARM1]) {
    int displayValue = alarm1H * 100 + alarm1M;
    clockDisplay.print(displayValue, DEC);
    if (alarm1H == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (alarm1M < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    clockDisplay.writeDigitRaw(2, 0x06);
  }
  else if (!btn[ALARM2]) {
    int displayValue = alarm2H * 100 + alarm2M;
    clockDisplay.print(displayValue, DEC);
    if (alarm2H == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (alarm2M < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    clockDisplay.writeDigitRaw(2, 10);
  }
  else {
    clockDisplay.clear();
  }
}

void readBtn() {
  if ((readTime - millis()) > 60) {
    for (int x = 0; x < BUTTONS; x++) {
      btn[x] = mcp.digitalRead(x);
    }
  }
}

void updateBtn() {
  for (int x = 0; x < BUTTONS; x++) {
    lbtn[x] = btn[x];
  }
}

void pharseData(String data) {
  data.toUpperCase();
  int nrCmd = getNumberOfCommands(data);
  String splitData[nrCmd][2];
  int lastCmd = 0;
  int index = 0;
  for (int x = 0; x < data.length(); x++) {
    if (data.substring(x, x + 1) == "#") {
      String temp = data.substring(lastCmd, x);
      for (int i = 0; i < temp.length(); i++) {
        if (temp.substring(i, i + 1) == ";") {
          splitData[index][0] = temp.substring(0, i);
          splitData[index][1] = temp.substring(i + 1);
          index++;
          break;
        }
      }
    }
  }
  for (int x = 0; x < nrCmd; x++) {
    String cmd = splitData[x][0];
    if (cmd == "GET ALL") {
      printAll();
    }
    else if (cmd == "TIME1") {
      setAlarm(1, splitData[x][1]);
    }
    else if (cmd == "TIME2") {
      setAlarm(2, splitData[x][1]);
    }
    else if (cmd == "DAYS1") {
      setAlarmDays(1, splitData[x][1]);
    }
    else if (cmd == "DAYS2") {
      setAlarmDays(2, splitData[x][1]);
    }
  }
}

int getNumberOfCommands(String in) {
  int nr = 0;
  for (int x = 0; x < in.length(); x++) {
    if (in.substring(x, x + 1) == "#") {
      nr++;
    }
  }
  return nr;
}

void printAll() {
  String sendData = "$";
  sendData += getAlarmTime();
  sendData += getAlarmDays();
  sendData += "%";
  conn.print(sendData);
}

String getAlarmTime() {
  String ret = "#TIME1;";
  if (alarm1H < 10) {
    ret += "0" + alarm1H;
  }
  else {
    ret += alarm1H;
  }
  if (alarm1M < 10) {
    ret += "0" + alarm1M;
  }
  else {
    ret += alarm1M;
  }
  ret += "#TIME2;";
  if (alarm2H < 10) {
    ret += "0" + alarm2H;
  }
  else {
    ret += alarm2H;
  }
  if (alarm2M < 10) {
    ret += "0" + alarm2M;
  }
  else {
    ret += alarm2M;
  }
  return ret;
}

String getAlarmDays() {
  String ret = "#DAYS1;";
  for (int x = 0; x < 7; x++) {
    switch (x) {
      case 6:
        if (alarm1Days[x]) {
          ret += "1";
        }
        else {
          ret += "0";
        }
        break;
      default:
        if (alarm1Days[x]) {
          ret += "1,";
        }
        else {
          ret += "0,";
        }
        break;
    }
  }
  ret += "#DAYS2;";
  for (int x = 0; x < 7; x++) {
    switch (x) {
      case 6:
        if (alarm2Days[x]) {
          ret += "1";
        }
        else {
          ret += "0";
        }
        break;
      default:
        if (alarm2Days[x]) {
          ret += "1,";
        }
        else {
          ret += "0,";
        }
        break;
    }
  }

  return ret;
}

void setAlarm(int alarmNr, String timeData) {
  switch (alarmNr) {
    case 1:
      for (int x = 0; x < timeData.length(); x++) {
        if (timeData.substring(x, x + 1) == ":") {
          alarm1H = timeData.substring(0, x).toInt();
          alarm1M = timeData.substring(x + 1).toInt();
          break;
        }
      }
      break;
    case 2:
      for (int x = 0; x < timeData.length(); x++) {
        if (timeData.substring(x, x + 1) == ":") {
          alarm2H = timeData.substring(0, x).toInt();
          alarm2M = timeData.substring(x + 1).toInt();
          break;
        }
      }
      break;
  }
}

void setAlarmDays(int alarmNr, String timeData) {
  int index = 0;
  switch (alarmNr) {
    case 1:
      for (int x = 0; x < timeData.length(); x++) {
        if (timeData.substring(x, x + 1) == ":") {
          if (timeData.substring(x - 1, x) == "1") {
            alarm1Days[index] = true;
          }
          else {
            alarm1Days[index] = false;
          }
          index++;
        }
      }
      break;
    case 2:
      for (int x = 0; x < timeData.length(); x++) {
        if (timeData.substring(x, x + 1) == ":") {
          if (timeData.substring(x - 1, x) == "1") {
            alarm2Days[index] = true;
          }
          else {
            alarm2Days[index] = false;
          }
          index++;
        }
      }
      break;
  }
}

void setAlarms() {
  Alarm.free(alarm1);
  Alarm.free(alarm2);
  for (int x = 0; x < 7; x++) {
    if (alarm1Days[x]) {
      alarm1 = Alarm.alarmRepeat(dow[x], alarm1H, alarm1M, 0, setOffAlarm);
    }
    if (alarm2Days[x]) {
      alarm2 = Alarm.alarmRepeat(dow[x], alarm2H, alarm2M, 0, setOffAlarm);
    }
  }
}

void setOffAlarm() {
  AlarmID_t a = Alarm.getTriggeredAlarmId();
  if (a == alarm1) {
    alarm1On = true;
  }
  else if (a == alarm2) {
    alarm2On = true;
  }
  playAlarm();
}

void getHttpReq() {
  WiFiClient httpClient = http.available();
  if (httpClient) {
    Serial.println("New client");
    httpClient.println("HTTP/1.1 200 OK");
    httpClient.println("Content-Type: text/html");
    httpClient.println();
    httpClient.println("<HTML>");
    httpClient.println("<HEAD>");
    httpClient.println("<TITLE>Alarm clock status</TITLE>");
    httpClient.println("</HEAD>");
    httpClient.println("<BODY>");
    httpClient.println("<H1>Alarm clock status:</H1>");
    httpClient.println(getHttpAlarmStatus(1));
    httpClient.println(getHttpAlarmStatus(2));
    httpClient.println("</BODY>");
    httpClient.println("</HEAD>");
    httpClient.println("</HTML>");
    httpClient.stop();
  }
}

String getHttpAlarmStatus(int alarmNr) {
  String ret = "<H4>";
  bool noDay = true;
  switch (alarmNr) {
    case 1:
      ret += "Alarm 1: ";
      if (alarm1H < 10) {
        ret += "0";
        ret += alarm1H;
      }
      else {
        ret += alarm1H;
      }
      ret += ":";
      if (alarm1M < 10) {
        ret += "0";
        ret += alarm1M;
      }
      else {
        ret += alarm1M;
      }
      ret += "<br>";
      for (int x = 0; x < 7; x++) {
        if (alarm1Days[x]) {
          ret += daysInWeek[x];
          ret += "  ";
          noDay = false;
        }
      }
      if (noDay) {
        ret += "Alarm set for no days";
      }
      break;
    case 2:
      ret += "Alarm 2: ";
      if (alarm2H < 10) {
        ret += "0";
        ret += alarm2H;
      }
      else {
        ret += alarm2H;
      }
      ret += ":";
      if (alarm2M < 10) {
        ret += "0";
        ret += alarm2M;
      }
      else {
        ret += alarm2M;
      }
      ret += "<br>";
      for (int x = 0; x < 7; x++) {
        if (alarm2Days[x]) {
          ret += daysInWeek[x];
          ret += "  ";
          noDay = false;
        }
      }
      if (noDay) {
        ret += "Alarm set for no days";
      }
      break;
  }
  ret += "</H4>";
  return ret;
}

