/*
 * NodeMCU esp8266 - Data sender
 * 
 * Pinout
 * 
 * D6 -> Arduino 8
 * D5 -> Arduino 9
 * 
 * 
 * NodeMCU and Arduino need to share GND
 * 
 */

#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "./lib/web/config.h"

const char* software_version = "0.0.1";
const char* host = "wasser.datenschule.de";
const char* url = "/api/v1/post-sensor-data";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "CF C4 73 BA 5B 14 5C 2A 91 3A AA 5B 89 B5 1B 3A FF 37 2E 57";
const int networkCredAddress = 10;
const String networkCredSeparator = "####";
const int networkCredSeparatorLength = networkCredSeparator.length();

uint32_t chipid;
String mac_address;
String ssid, pw;
bool networkCredSet = false;

/*
 * Function declarations
 */
void writeString(char add, String data);
String read_String(char add);
void clearStorage();
String getJsonObjectString(String type, float val);
String makePostString(String sensor, int pin, String sensordata);
void goIntoAPMode();
void goIntoSTAMode();
void handleRoot();
void handleConfig();
void handlePostConfig();
void handleGetStorage(); // debugging route, will ideally be deleted
void handleReset();

ESP8266WebServer server(80);
SoftwareSerial swSer;

void setup() {
  delay(300);
  swSer.begin(19200, SWSERIAL_8N1, D5, D6, false, 95, 11);
  Serial.begin(19200);
  EEPROM.begin(512);
  
  chipid = ESP.getChipId();
  mac_address = WiFi.macAddress();
  WiFi.mode(WIFI_OFF);
  delay(500);

  // read form EEPROM, check if wifi credentials have been set
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
    goIntoSTAMode();
  } else {
    goIntoAPMode();
  }

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/post_config", handlePostConfig);
  server.on("/get_storage", handleGetStorage); // for debugging, should be removed
  server.on("/reset", handleReset);
  server.begin();
}

void loop() {
  server.handleClient(); 

  if (networkCredSet) { 
    if (swSer.available() > 0) {
      // reading from Serial
      String data = swSer.readStringUntil('\n');
      char charBuf[data.length()];
      StaticJsonDocument<900> jsonBuffer;
      DeserializationError error;
      error = deserializeJson(jsonBuffer, data);
      if (error) {
        Serial.println(error.c_str());
        return;
      }

      // opening HTTPS connection
      WiFiClientSecure client;
      Serial.print("connecting to ");
      Serial.println(host);
      Serial.printf("Using fingerprint '%s'\n", fingerprint);
      client.setFingerprint(fingerprint);
      if (!client.connect(host, httpsPort)) {
        Serial.println("Secure connection failed");
        return;
      }

      Serial.print("POSTing");
      // post for every available value
      JsonArray arr = jsonBuffer[0].as<JsonArray>();
      for ( int i = 0; i < arr.size(); i++) {
        String sensor = jsonBuffer[0][i]["sensor"];
        int pin = jsonBuffer[0][i]["pin"];
        String type = jsonBuffer[0][i]["type"];
        float value = jsonBuffer[0][i]["value"];
        String json = getJsonObjectString(type, value);
        client.print(makePostString(sensor, pin, json));
        //Serial.print(makePostString(sensor, pin, json));  
      }
    }
  }
}

/*
 * JSON PREP
 */

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

/*
 * WIFI SETUP
 */
void goIntoAPMode() {
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

void goIntoSTAMode() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname("Wassersensor-" + String(chipid));     
  WiFi.begin(ssid, pw);
  Serial.print("Connecting");
  int try_counter = 0;
  while (WiFi.status() != WL_CONNECTED && try_counter < 20) {
    delay(500);
    Serial.print(".");
    try_counter++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(WiFi.hostname());
  } else {
    Serial.println("");
    Serial.println("Could not connect to WiFi, opening access point");
    goIntoAPMode();
  } 
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
  server.send(200, "application/json", "{\"notice\": \"Storage cleared\"}"); 
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
