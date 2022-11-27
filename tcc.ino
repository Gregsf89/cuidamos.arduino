#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

static const int RXPin = 8, TXPin = 9;
static const int baud = 9600;
String apn = "timbrasil.br";
String apn_u = "tim";
String apn_p = "tim";
String url = "https://8e24-2804-7f0-b9c0-3103-2909-40ba-b72f-1a97.sa.ngrok.io/api/device/log";

TinyGPSPlus gps;
TinyGPSTime time;
TinyGPSDate date;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  DynamicJsonDocument doc(1024);

  doc["device_id"] = 1;
  doc["latitude"]  = gps.location.lat();
  doc["longitude"] = gps.location.lng();
  doc["altitude"]  = gps.altitude.meters();
  doc["date"]      = String(date.year()) + "-" + String(date.month()) + "-" + String(date.day());
  doc["time"]      = String(time.hour()) + ":" + String(time.minute()) + ":" + String(time.second());
  doc["speed"]     = gps.speed.kmph();
  doc["accuracy"]  = gps.hdop.value();

  serializeJson(doc, Serial);

  Serial.begin(baud);
  ss.begin(baud);
}

void loop() {
  smartDelay(5000);
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void gsm_http_post(String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/json");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPSSL=1");
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  ss.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (ss.available()) {
      Serial.write(ss.read());
    }
  }
  Serial.println();
}