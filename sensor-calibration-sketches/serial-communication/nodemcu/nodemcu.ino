#include <SoftwareSerial.h>
SoftwareSerial s(D2,D3); // (Rx, Tx)
#include <ArduinoJson.h>
 
void setup() {
  Serial.begin(9600);
  s.begin(4800);
  Serial.print("start");
}
 
void loop() {
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

  } else {
    delay(50);  
  }  
}
