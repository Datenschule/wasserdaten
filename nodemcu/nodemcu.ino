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
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include "./lib/web/config.h"

#define sendingInterval (1000UL * 60 * 5)

const char* software_version = "0.0.1";
const char* host = "wasser.datenschule.de";
char* url = "/api/v1/post-sensor-data";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "CF C4 73 BA 5B 14 5C 2A 91 3A AA 5B 89 B5 1B 3A FF 37 2E 57";
uint32_t chipid;
String mac_address;

const int networkCredAddress = 10;
bool networkCredSet = false;
const String networkCredSeparator = "####";
const int networkCredSeparatorLength = networkCredSeparator.length();


//**
// Function declarations
//**
void writeString(char add, String data);
String read_String(char add);
void clearStorage();

String getJsonObjectString(String type, float val);
String makePostString(String sensor, int pin, String sensordata);

void handleRoot();
void handleConfig();
void handlePostConfig();
void handleGetStorage(); // debugging route, will ideally be deleted
void handleReset();

ESP8266WebServer server(80);
SoftwareSerial s(D2,D3); // (Rx, Tx)

void setup() {
  delay(500);
  s.begin(4800); // same as Arduino
  Serial.begin(9600);
  EEPROM.begin(512);
  
  chipid = ESP.getChipId();
  mac_address = WiFi.macAddress();
  WiFi.hostname("Wassersensor-" + String(chipid));
  WiFi.mode(WIFI_OFF);
  delay(500);

  String ssid;
  String pw;
  String data;
  
  data = read_String(networkCredAddress);
  int pos = data.indexOf(networkCredSeparator);
  ssid = data.substring(0, pos);
  pw = data.substring(pos + networkCredSeparatorLength);
  
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

        WiFiClientSecure client;
        Serial.print("connecting to ");
        Serial.println(host);
        Serial.printf("Using fingerprint '%s'\n", fingerprint);
        client.setFingerprint(fingerprint);
      
        if (!client.connect(host, httpsPort)) {
          Serial.println("connection failed");
          return;
        }

        Serial.print("POSTing");

        float t = jsonBuffer["temperature"];
        float h = jsonBuffer["humidity"];
        if (!isnan(t) && !isnan(h)) {
          String tJson = getJsonObjectString("temperature", t);
          String hJson = getJsonObjectString("humidity", h);
          Serial.print(makePostString("DHT22", 8, tJson + "," + hJson));
          client.print(makePostString("DHT22", 8, tJson + "," + hJson));
        }

        float m = jsonBuffer["moisture"];
        if (!isnan(m)) {
          String mJson = getJsonObjectString("moisture", m);
          Serial.print(makePostString("CSMS", 14, mJson));
          client.print(makePostString("CSMS", 14, mJson));
        }

        sendingTime = millis();
      }
    }
  }
}

String getJsonObjectString(String type, float val) {
  return "{\"value_type\": \""+ type +"\", \"value\": "+ val +"}";
}

String makePostString(String sensor, int pin, String sensordata) {
  String content = "{\"sensor\": \""+ sensor +"\", "+
             "\"software_version\": \""+ software_version +"\", "+
             "\"sensordatavalues\": ["+ sensordata +"]}";
  
  return String("POST ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: WaterLab-ESP8266\r\n" +
                "Content-Type: application/json\r\n" +
                "X-Sensor: esp8266-"+ mac_address +"\r\n" + 
                "X-Pin: "+ pin +"\r\n" +
                "Content-Length: "+ content.length() +"\r\n\r\n" + 
                content + "\r\n\r\n";
                // connection close??
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
      writeString(networkCredAddress, s1 + networkCredSeparator + s2);
      server.send(200, "application/json", "{\"success\": \"SSID and password saved\"}");
  }
  server.send(400, "application/json", "{\"error\": \"Not right\"}");
} 

void handleGetStorage() {
  String ssidData;
  String pwData;
  String data;
  data = read_String(networkCredAddress);
  int pos = data.indexOf(networkCredSeparator);
  if (pos) {
    ssidData = data.substring(0, pos);
    pwData = data.substring(pos + networkCredSeparatorLength);
    server.send(200, "application/json", "{\"ssid\": \""+ ssidData +"\", \"pw\": \"" + pwData +"\"}"); 
  } else {
    Serial.println("Error: Could not find position of separator");  
  }
}

void handleReset() {
  clearStorage();
  server.send(200, "application/json", "{\"notice\": \"You won't even see this\"}"); 
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
