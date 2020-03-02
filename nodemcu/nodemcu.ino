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

#define FIVEMIN (1000UL * 60 * 1)
unsigned long rolltime = millis() + FIVEMIN;

//**
// Change these to what you need!
//**
const String host = "10.10.10.209:3000"; // local IP of sinatraserver
const String post_endpoint = "http://"+ host + "/send-data"; // also sinatraserver

const String software_version = "0.0.1";
const int sampling_rate = 9600;

const int networkCredAddress = 10;
bool networkCredSet = false;
String networkCredSeparator = "####";
int networkCredSeparatorLength = networkCredSeparator.length();
uint32_t chipid;

void writeString(char add, String data);
String read_String(char add);
void clearStorage();

String getTimeStampString();

String packPayload();
void sendData();
void handleRoot();
void handleConfig();
void handlePostConfig();
void handleGetStorage(); // debugging route, will ideally be deleted
void handleReset();


void setup() {
  delay(1000);
  Serial.begin(sampling_rate);
  timeClient.begin();
  dht.begin();

  chipid = ESP.getChipId();
  EEPROM.begin(512);
  WiFi.hostname("Wassersensor-" + String(chipid));
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);

  // this does not work
  // maybe only writing a 1 byte flag will do better?
  String ssid;
  String pw;
  String data;
  
  data = read_String(networkCredAddress);
  int pos = data.indexOf("####");
  ssid = data.substring(0, pos);
  pw = data.substring(pos + 4);

  if (ssid != "" && pw != "") {
      networkCredSet = true;
      Serial.println("Found cred, looks legit");
  }

  if (networkCredSet) {
    WiFi.mode(WIFI_STA);     
    WiFi.begin(ssid, pw);

    Serial.print("Connecting");
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    Serial.print("Hostname: ");
    Serial.println("Wassersensor-" + String(chipid));
  } else {
    // set default values
    ssid = "sensor-ap";
    pw = "password";
    WiFi.mode(WIFI_AP);  
    WiFi.softAP(ssid, pw);
    Serial.println("");
    Serial.print("AccessPoint: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  // defining routes for webserver
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/post_config", handlePostConfig);
  server.on("/get_storage", handleGetStorage);
  server.on("/reset", handleReset);
  server.begin();
}


void loop() {
  if (networkCredSet) {
    timeClient.update();

    if((long)(millis() - rolltime) >= 0) {
      sendData();
      rolltime += FIVEMIN;
    }
  } else {
    //Serial.println("no connection, indleing");  
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
      writeString(networkCredAddress, s1 + "####" + s2);
      server.send(200, "application/json", "{\"success\": \"SSID and password saved\"}");
  }
  server.send(400, "application/json", "{\"error\": \"Not right\"}");
} 

void handleGetStorage() {
  String ssidData;
  String pwData;
  String data;
  data = read_String(networkCredAddress);
  int pos = data.indexOf("####");
  if (pos) {
    ssidData = data.substring(0, pos);
    pwData = data.substring(pos + 4);
    server.send(200, "application/json", "{\"ssid\": \""+ ssidData +"\", \"pw\": \"" + pwData +"\"}"); 
  } else {
    Serial.println("Error: Could not find position of separator");  
  }
}

void handleReset() {
  clearStorage();
  server.send(200, "application/json", "{\"notice\": \"Storage cleared\"}"); 
}


void sendData() {
  HTTPClient http;
  http.begin(post_endpoint);
  http.addHeader("X-Token", "whatisthis");
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("X-Pin", "?");
  http.addHeader("X-Sensor", "esp8266-" + String(chipid));
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
  root["sensor"] = chipid;
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

void clearStorage() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }  
  EEPROM.commit();
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
