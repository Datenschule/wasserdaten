#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <DHT.h>

#include "./lib/web/config.h"

#define DHTPIN 13 // aka D7 on nodemcu v2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

ESP8266WebServer server(80);

//**
// Change these to what you need!
//**
const String host = "10.10.10.209:3000"; // local IP of sinatraserver
const String post_endpoint = "http://"+ host + "/send-data"; // also sinatraserver

const String software_version = "0.0.1";
const int sampling_rate = 9600;

//todo
// do not instantiate here, we need to fill in later 
const char* ssid = "sensor-ap";
const char* password = "password";
//const char *ssid = "<3";  //ENTER YOUR WIFI SETTINGS
//const char *password = "datalove";

#define FIVEMIN (1000UL * 60 * 1)
unsigned long rolltime = millis() + FIVEMIN;

// EEPROM stuff
void writeString(char add, String data);
String read_String(char add);
// ntp time stuff
String getTimeStampString();
// web stuff
String packPayload();
void sendData();
void handleRoot();
void handleConfig();
void handlePostConfig();
void handleGetStorage(); // debugging route, will ideally be deleted


void setup() {
  delay(1000);
  Serial.begin(sampling_rate);
  timeClient.begin();
  dht.begin();
  EEPROM.begin(512);

  // we probs need to read from eeprom here and go from there
  
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  
  // if no ssid and pw are set,
  // then go WIFI_AP mode
  WiFi.mode(WIFI_AP);  
  WiFi.softAP(ssid, password);
  Serial.println("");
  Serial.print("AccessPoint: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // else WIFI_STA mode
  //WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  //WiFi.begin(ssid, password);     //Connect to your WiFi router

  //Serial.print("Connecting");
  // Wait for connection
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.print("Connected to ");
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // defining routes for webserver
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/post_config", handlePostConfig);
  server.on("/get_storage", handleGetStorage);
  server.begin();
}


void loop() {
  timeClient.update();

  if((long)(millis() - rolltime) >= 0) {
    sendData();
    rolltime += FIVEMIN;
  }
  server.handleClient(); 
  delay(100);
}

//
// ROUTE HANDLING
//
void handleRoot() {
  String webPage = packPayload();
  server.send(200, "application/json", webPage);
}

void handleConfig() {
  server.send(200, "text/html", CONFIG_page);
}

void handlePostConfig() {
  const size_t capacity = JSON_OBJECT_SIZE(2) + 70;
  DynamicJsonDocument doc(capacity);  
  deserializeJson(doc, server.arg(0));
  const char* new_ssid = doc["new_ssid"];
  const char* new_password = doc["new_password"];

  if (new_ssid && new_password) {
      String s1 = String(new_ssid);
      String s2 = String(new_password);
      writeString(10, s1 + "####" + s2);
      server.send(200, "application/json", "{\"success\": \"SSID and password saved\"}");
  }
  server.send(400, "application/json", "{\"error\": \"Not right\"}");
} 

void handleGetStorage() {
  String ssidData;
  String pwData;
  String data;
  data = read_String(10);
  int pos = data.indexOf("####");
  ssidData = data.substring(0, pos);
  pwData = data.substring(pos + 4);
  server.send(200, "application/json", "{\"ssid\": \""+ ssidData +"\", \"pw\": \"" + pwData +"\"}");
}


void sendData() {
  HTTPClient http;
  http.begin(post_endpoint);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(packPayload());
  String payload = http.getString();
  if (Serial) {
    Serial.println(httpCode);
    Serial.println(payload);
  }
  http.end();
}

String packPayload() {
  String webPage;
  StaticJsonDocument<500> jsonBuffer;
  JsonObject root = jsonBuffer.createNestedObject("data");

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  root["software_version"] = software_version;
  root["timestamp"] = getTimeStampString();
  root["sampling_rate"] = sampling_rate;
  root["sensor"] = "DHT22";
  JsonArray measurements = root.createNestedArray("sensordatavalues");
  JsonObject measurement1 = measurements.createNestedObject();
  measurement1["value_type"] = "temperature";
  measurement1["value"] = t;
  JsonObject measurement2 = measurements.createNestedObject();
  measurement2["value_type"] = "humidity";
  measurement2["value"] = h;

  serializeJson(root, webPage);
  return webPage;
}

//
// EEPROM helpers
//
void writeString(char add, String data) {
  int _size = data.length();
  int i;
  for(i=0; i<_size; i++) {
    EEPROM.write(add+i, data[i]);
  }
  EEPROM.write(add+_size,'\0');
  EEPROM.commit();
}
 
 
String read_String(char add) {
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k= EEPROM.read(add);
  while(k != '\0' && len < 500) {    
    k = EEPROM.read(add+len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

//
// TIME HELPER
// https://github.com/arduino-libraries/NTPClient/issues/36#issuecomment-439631673
String getTimeStampString() {
   time_t rawtime = timeClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr;
}
