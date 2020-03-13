/*
 * NodeMCU esp8266 - Data sender
 * 
 * Pinout
 * 
 * D2 (Rx) -> Arduino 2 (Tx)
 * D3 (Tx) -> Arduino 3 (Rx)
 * 
 * 
 * NodeMCU and Arduino need to share GND
 */

#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "./lib/web/config.h"

#define sendingInterval (1000UL * 60 * 5)

ESP8266WebServer server(80);
SoftwareSerial s(D2,D3); // (Rx, Tx)

//**
// Change these to what you need!
//**
const String host = "192.168.178.39:8000"; // local IP of server
const String post_endpoint = "http://"+ host + "/api/v1/post-sensor-data"; // also server

const String software_version = "0.0.1";

const int networkCredAddress = 10;
bool networkCredSet = false;
String networkCredSeparator = "####";
int networkCredSeparatorLength = networkCredSeparator.length();
uint32_t chipid;
String mac_address;

//**
// Function declarations
//**
void writeString(char add, String data);
String read_String(char add);
void clearStorage();
void sendTemperature(float t, float h);
void sendMoisture(int m);
void handleRoot();
void handleConfig();
void handlePostConfig();
void handleGetStorage(); // debugging route, will ideally be deleted
void handleReset();


void setup() {
  delay(500);
  s.begin(4800); // same as Arduino
  Serial.begin(9600);
  EEPROM.begin(512);
  
  chipid = ESP.getChipId();
  mac_address = WiFi.macAddress();
  WiFi.hostname("Wassersensor-" + String(chipid));
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(500);

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
    Serial.println("WaterLab-" + String(chipid));
  } else {
    ssid = "WaterLab";
    pw = "password";
    WiFi.mode(WIFI_AP);  
    WiFi.softAP(ssid, pw);
    Serial.println("");
    Serial.print("AccessPoint: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/post_config", handlePostConfig);
  server.on("/get_storage", handleGetStorage);
  server.on("/reset", handleReset);
  server.begin();
}


void loop() {
  server.handleClient(); 
  
  static unsigned long sendingTime = millis();
  if (networkCredSet) {
    if(millis()- sendingTime > sendingInterval) {    
      if (s.available() > 0) {
        String data = s.readStringUntil('\n');
        //Serial.println(data);
        char charBuf[data.length()];
        data.toCharArray(charBuf, data.length());
        
        StaticJsonDocument<500> jsonBuffer;
        DeserializationError error;
        error = deserializeJson(jsonBuffer, charBuf, DeserializationOption::NestingLimit(10));
      
        if (error) {
          Serial.println(error.c_str());
          return;
        }
        serializeJsonPretty(jsonBuffer, Serial);

        float t = jsonBuffer["temperature"];
        float h = jsonBuffer["humidity"];
        if (!isnan(t) && !isnan(h)) {
          sendTemperature(t, h);
        }

        int m = jsonBuffer["moisture"];
        if (!isnan(m)) {
          sendMoisture(m);
        }
        
        sendingTime = millis();
      }
    }
  }
}

/*
 * ROUTE HANDLING
 */
void handleRoot() {
  String webPage = "{\"notice\": \"Nothing here atm\"}";
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
  server.send(200, "application/json", "{\"notice\": \"You won't even see this\"}"); 
}

void sendTemperature(float t, float h) {
  HTTPClient http;
  http.begin(post_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Sensor", "esp8266-" + mac_address);
  http.addHeader("X-Pin", "8"); // should be config var
  
  String payload;
  StaticJsonDocument<300> jsonBuffer;
  JsonObject root = jsonBuffer.createNestedObject("data");
  root["software_version"] = software_version;
  root["sensor"] = "DHT22";
  JsonArray measurements = root.createNestedArray("sensordatavalues");
  JsonObject measurement1 = measurements.createNestedObject();
  measurement1["value_type"] = "temperature";
  measurement1["value"] = t;
  JsonObject measurement2 = measurements.createNestedObject();
  measurement2["value_type"] = "humidity";
  measurement2["value"] = h;
  serializeJson(root, payload);

  int httpCode = http.POST(payload);
  http.end();
}

void sendMoisture(int m) {
  HTTPClient http;
  http.begin(post_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Sensor", "esp8266-" + mac_address);
  http.addHeader("X-Pin", "14"); // should be config var
  
  String payload;
  StaticJsonDocument<300> jsonBuffer;
  JsonObject root = jsonBuffer.createNestedObject("data");
  root["software_version"] = software_version;
  root["sensor"] = "CSMS";
  JsonArray measurements = root.createNestedArray("sensordatavalues");
  JsonObject measurement1 = measurements.createNestedObject();
  measurement1["value_type"] = "moisture";
  measurement1["value"] = m;
  serializeJson(root, payload);

  int httpCode = http.POST(payload);
  http.end();
}


/*
 * EEPROM helpers
 */
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
