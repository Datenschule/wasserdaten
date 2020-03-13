#include <DHT.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial s(3,2); // (Rx, Tx)

#define DHTPIN 8     
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
 
void setup() {
  Serial.begin(9600);
  s.begin(4800);  
  dht.begin();
}

 
void loop() { 
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
  //float f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t)) {
    //Serial.println("isnan, skip");
    return;
  }

  StaticJsonDocument<300> jsonBuffer;
  JsonObject root = jsonBuffer.createNestedObject("data");
  root["temperature"] = t;
  root["humidity"] = h;

  char buf[300];
  serializeJson(root, buf);

  //serializeJsonPretty(root, Serial);
  
  if(s.available()>0) {  
    //serializeJson(root, s);
    
    s.println(buf);
    
    //s.print(t);
    //s.print(",");
    //s.println(h);
  }
}
