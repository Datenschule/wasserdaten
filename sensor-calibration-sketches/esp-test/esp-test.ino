//#include <StreamUtils.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#if defined(ESP8266) && !defined(D5)
#define D5 (14)
#define D6 (12)
#define D7 (13)
#define D8 (15)
#endif

SoftwareSerial swSer; // (Rx, Tx)

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  swSer.begin(19200, SWSERIAL_8N1, D5, D6, false, 95, 11);
}

char* foo;

void loop() { // run over and over
  
  if (swSer.available() > 0) {    
    //Serial.write(swSer.read());


    Serial.println("foo!");
    String data = swSer.readStringUntil('\n');
    char charBuf[data.length()];
    //data.toCharArray(charBuf, data.length());

    //Serial.println(data);

    StaticJsonDocument<900> jsonBuffer;
    DeserializationError error;
    error = deserializeJson(jsonBuffer, data);
  
    if (error) {
      Serial.println(error.c_str());
      return;
    }
    //serializeJsonPretty(jsonBuffer, Serial);
    JsonArray arr = jsonBuffer[0].as<JsonArray>();

    Serial.print(arr.size());
    for ( int i = 0; i < arr.size(); i++) {
      String type = jsonBuffer[0][i]["type"];
      Serial.print(type);  
    }

    
  }
}
