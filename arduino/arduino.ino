/*
 * Arduino, Sensor Data Collector
 * 
 * Uno pinout
 * 
 * 3 (Tx) -> NodeMCU D3 (Rx)
 * 2 (Rx) -> NodeMcu D2 (Tx)
 * 
 * 8 -> DHT22
 * 
 * Arduino and NodeMCU need a shared GND
 */


#include <DHT.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial s(3,2); // (Rx, Tx)

#define DHTPIN 8     
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MOISTUREPIN A0

#define samplingInterval 100
#define ArrayLenth  40
int tempArray[ArrayLenth];
int tempArrayIndex=0;
int humArray[ArrayLenth];
int humArrayIndex=0;
int moiArray[ArrayLenth];
int moiArrayIndex=0;
 
void setup() {
  //Serial.begin(9600);
  s.begin(4800);  
  dht.begin();
}

 
void loop() { 
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float temperature, humidity;
  static int moisture;
  
  if(millis()-samplingTime > samplingInterval) {
    tempArray[tempArrayIndex++]= dht.readTemperature();
    if(tempArrayIndex==ArrayLenth) tempArrayIndex=0;
    temperature = avergearray(tempArray, ArrayLenth);

    humArray[humArrayIndex++]= dht.readHumidity();
    if(humArrayIndex==ArrayLenth) humArrayIndex=0;
    humidity = avergearray(humArray, ArrayLenth);

    moiArray[moiArrayIndex++]= analogRead(MOISTUREPIN);
    if(moiArrayIndex==ArrayLenth) moiArrayIndex=0;
    moisture = avergearray(moiArray, ArrayLenth);

    StaticJsonDocument<300> jsonBuffer;
    JsonObject root = jsonBuffer.createNestedObject("data");

    root["temperature"] = temperature;
    root["humidity"] = humidity;
    root["moisture"] = moisture;

    char buf[300];
    serializeJson(root, buf);
    //serializeJsonPretty(root, Serial);

    if(s.available()>0) {  
      s.println(buf);
    }
    
    samplingTime=millis();
  }
}



double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number<5) {   //less than 5, calculated directly statistics
    for (i=0;i<number;i++) {
      amount += arr[i];
    }
    avg = amount/number;
    return avg;
  } else {
    if(arr[0]<arr[1]){
      min = arr[0];
      max=arr[1];
    } else {
      min=arr[1];
      max=arr[0];
    }
    for (i=2; i<number; i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      } else {
        if (arr[i]>max) {
          amount+=max;    //arr>max
          max=arr[i];
        } else {
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
