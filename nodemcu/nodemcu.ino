#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <DHT.h>

#define DHTPIN 13 // aka D7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

//**
// Change these to what you need!
//**
const char *ssid = "<3";  //ENTER YOUR WIFI SETTINGS
const char *password = "datalove";
const String host = "10.10.10.38:3000";
const String post_endpoint = "http://"+ host + "/send-data";

const String software_version = "0.0.1";
const int sampling_rate = 9600;

ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

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

void handleRoot() {
  String webPage = packPayload();
  server.send(200, "application/json", webPage);
}


void setup() {
  delay(1000);
  Serial.begin(sampling_rate);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  timeClient.begin();
  dht.begin();

  // defining routes for webserver
  server.on("/", handleRoot);
  server.begin();

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void loop() {
  timeClient.update();
  
  // posting to data server
  HTTPClient http;
  http.begin(post_endpoint);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(packPayload());
  String payload = http.getString();
  Serial.println(httpCode);
  Serial.println(payload);
  http.end();

  // web server
  // see routes set in setup()
  server.handleClient();
  
  delay(200);
}
