#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <Adafruit_LEDBackpack.h>
#include "Adafruit_GFX.h"
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23008.h"


#define TIME_24_HOUR  true
#define DISPLAY_ADDRESS 0x70
#define SFX_TX 12
#define SFX_RX 13
#define SFX_RST 14

#define S_ADD 0
#define S_HOUR 4
#define S_MIN 5
#define S_DEL 1
#define S_DAY 2
#define S_TIME 3

#define TIME 0
#define ALARM1 1
#define ALARM2 2
#define SNOOZE 3
#define ALARM_OFF 4
#define S_W_TIME 5

#define NUM_OF_ALARMS 2
#define NUM_OF_KEYWORDS 3


RTC_DS3231 rtc;
Adafruit_7segment clockDisplay = Adafruit_7segment();
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
Adafruit_MCP23008 mcp;

WiFiServer server (80);

AlarmID_t a1;
AlarmID_t a2;
AlarmID_t a3;
AlarmID_t a4;
AlarmID_t snoozeAlarm;
AlarmID_t updateTime;

bool alarmStatus[] = {false, false, false, false, false};
bool alarmDays[NUM_OF_ALARMS][7];
bool buttons[] = {true, true, true, true, true, true};
bool disp = false;
bool alarmTrig = false;

byte brightness = 12;

char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char* weekVal[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
char* keywords[] = {"del", "days", "time"};
const char* ssid = "Tussa_4277"; //"Telenor9813oss"; //"Inteno-78FC";
const char* password = "znid307160439";//"zyebwozltjqbr"; //"JNC9KIZZPR64";

int alarmHours[5] = {10, 8, 0, 0, 0};
int alarmMins[5] = {25, 10, 30, 0, 0};
int hours;
int minutes;
int days;
int nextDay;

String HTTP_req = "";

unsigned long showTime = 0;
unsigned long buttonRead = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  mcp.begin();
  for(int x = 0; x < 6; x++){
    mcp.pinMode(x, INPUT);
    mcp.pullUp(x, HIGH);
  }
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.setBrightness(brightness); //0-15
  for (int x = 0; x < NUM_OF_ALARMS; x++) {
    for (int y = 0; y < 7; y++) {
      alarmDays[x][y] = false;
    }
  }
  Serial.println();
  Serial.println();
  disableAlarms();
  Serial.println("Try to connect");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");
  server.begin();
  Serial.println(WiFi.localIP());
  ss.begin(9600);
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
}

void loop() {
  clockFun();
  clientFun();
  displayFun();
  buttonFun();
  disableAlarm();
}

void clockFun() {
  DateTime dt = rtc.now();
  hours = dt.hour();
  minutes = dt.minute();
  days = dt.dayOfTheWeek();
  nextDay = days == 6 ? 0 : days + 1;
}

void clientFun() {
  WiFiClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    boolean firstLine = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (firstLine) {
          HTTP_req += c;
        }
        if (c == '\n' && currentLineIsBlank) {
          if (HTTP_req.indexOf("favicon") > -1) {
            HTTP_req = "";
            break;
          }
          parseHTTP(HTTP_req);
          client.flush();
          Serial.println();
          Serial.println(HTTP_req);
          sendHTTPResponce(client);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
          firstLine = false;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    Alarm.delay(1);
  }
}

void disableAlarm(){
  if(alarmTrig && !buttons[ALARM_OFF]){
    sfx.stop();
    alarmTrig = false;
  }
  else if(alarmTrig && !buttons[SNOOZE]){
    sfx.stop();
    snoozeAlarm = Alarm.timerOnce(540, alarm);
    alarmTrig = false;
  }
}

void displayFun() {
  if (!buttons[TIME]) {
    int displayValue = hours * 100 + minutes;
    clockDisplay.print(displayValue, DEC);
    if (hours == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (minutes < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    showTime = millis() + 1000;
    disp = true;
  }
  else if (!buttons[ALARM1]) {
    int displayValue = alarmHours[0] * 100 + alarmMins[0];
    clockDisplay.print(displayValue, DEC);
    if (alarmHours[0] == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (alarmMins[0] < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    showTime = millis() + 1000;
    disp = true;
  }
  else if (!buttons[ALARM2]) {
    int displayValue = alarmHours[0] * 100 + alarmMins[0];
    clockDisplay.print(displayValue, DEC);
    if (alarmHours[0] == 0) {
      clockDisplay.writeDigitNum(1, 0);
      if (alarmMins[0] < 10) {
        clockDisplay.writeDigitNum(3, 0);
      }
    }
    showTime = millis() + 1000;
    disp = true;
  }
  else if (millis() > showTime) {
    if (disp) {
      clockDisplay.clear();
      disp = false;
    }
  }
  clockDisplay.writeDisplay();
}

void buttonFun() {
  if(millis() > buttonRead){
    for(int x = 0; x < 6; x++){
      buttons[x] = mcp.digitalRead(x);
      if(!buttons[x]){
        Serial.println("Button" + String(x, DEC) + " pressed");
      }
    }
    buttonRead = millis() + 500;
  }
}

void alarm() {
  alarmTrig = true;
  sfx.playTrack(1);
}

void sendHTTPResponce(WiFiClient cl) {
  cl.println("HTTP/1.1 200 OK");
  cl.println("Content-Type: text/html");
  cl.println("Connection: close");
  cl.println();
  // web page
  cl.println("<!DOCTYPE html>");
  cl.println("<meta charset=\"UTF-8\">");
  cl.println("<html>");
  cl.println("<head>");
  cl.println("<title>Alarm Clock</title>");
  cl.println("</head>");
  cl.println("<body>");
  cl.println("<h2>Alarm clock</h2>");
  cl.println("<fieldset>");
  cl.println("<legend>Set alarm</legend>");
  for (int x = 0; x < NUM_OF_ALARMS; x++) {
    if (alarmStatus[x]) {
      String timeVal = getTimeString(x);
      cl.println("<form action=\"/A" + String(x, DEC) + "\" method=\"GET\">");
      cl.println("<input type=\"time\" name=\"time\" value=\"" + timeVal + "\">");
      for (int i = 0; i < 7; i++) {
        if (alarmDays[x][i]) {
          cl.println("<input type=\"checkbox\" name=\"days\" value=\"" + String(weekVal[i]) + "\" checked>" + String(weekdays[i]));
        }
        else {
          cl.println("<input type=\"checkbox\" name=\"days\" value=\"" + String(weekVal[i]) + "\">" + String(weekdays[i]));
        }
      }
      cl.println("<input type=\"submit\" value=\"Set alarm\">");
      cl.println("<input type=\"submit\" name=\"del\" value=\"Delete\">");
      cl.println("</form>");
    }
  }
  cl.println("</fieldset>");
  cl.println("<form action=\"/add\">");
  cl.println("<input type=\"submit\" value=\"Add alarm\">");
  cl.println("</form>");
  cl.println("</body>");
  cl.println("</html>");
}

void disableAlarms() {
  Alarm.disable(a1);
  Alarm.disable(a2);
  Alarm.disable(a3);
  Alarm.disable(a4);
  Alarm.disable(snoozeAlarm);
}

void disableAlarm(int i) {
  switch (i) {
    case 0:
      Alarm.disable(a1);
      break;
    case 1:
      Alarm.disable(a2);
      break;
    case 2:
      Alarm.disable(a3);
      break;
    case 3:
      Alarm.disable(a4);
      break;
    case 4:
      Alarm.disable(snoozeAlarm);
      break;
  }
}

void parseHTTP(String req) {
  if (req.indexOf("/add") > -1) {
    addAlarm();
  }
  else {
    for (int x = 0; x < NUM_OF_ALARMS; x++) {
      String alarmKey = "/A" + String(x, DEC);
      if (req.indexOf(alarmKey) > -1) {
        if (req.indexOf("del") > -1) {
          alarmStatus[x] = false;
          break;
        }
        else {
          int timeIndex = req.indexOf("time=");
          alarmHours[x] = req.substring(timeIndex + 5, timeIndex + 7).toInt();
          alarmMins[x] = req.substring(timeIndex + 10, timeIndex + 12).toInt();
          for (int i = 0; i < 7; i++) {
            String daysKey = "days=" + String(weekVal[i]);
            if (req.indexOf(daysKey) > -1) {
              alarmDays[x][i] = true;
            }
            else {
              alarmDays[x][i] = false;
            }
          }
        }
      }
    }
  }
  updateAlarms();
}

void addAlarm() {
  for (int i = 0; i < NUM_OF_ALARMS; i++) {
    if (!alarmStatus[i]) {
      alarmStatus[i] = true;
      break;
    }
  }
}

void updateAlarms() {
  for (int i = 0; i < NUM_OF_ALARMS; i++) {
    if (alarmStatus[i]) {
      switch (i) {
        case 0:
          a1 = Alarm.alarmRepeat(alarmHours[0], alarmMins[0], 0, alarm);
          break;
        case 1:
          a2 = Alarm.alarmRepeat(alarmHours[1], alarmMins[1], 0, alarm);
          break;
        case 2:
          a3 = Alarm.alarmRepeat(alarmHours[2], alarmMins[2], 0, alarm);
          break;
        case 3:
          a4 = Alarm.alarmRepeat(alarmHours[3], alarmMins[3], 0, alarm);
          break;
      }
    }
  }
}

String getTimeString(int index) {
  String s = "";
  if (alarmHours[index] < 10) {
    s += "0" + String(alarmHours[index], DEC);
  }
  else {
    s += String(alarmHours[index], DEC);
  }
  s += ":";
  if (alarmMins[index] < 10) {
    s += "0" + String(alarmMins[index], DEC);
  }
  else {
    s += String(alarmMins[index], DEC);
  }
  return s;
}

