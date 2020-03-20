/*
 * Arduino, Sensor Data Collector
 * 
 * Uno pinout
 * 
 * 9 (Tx) -> NodeMCU D3 (Rx)
 * 8 (Rx) -> NodeMcu D2 (Tx)
 * 
 * 
 * A0 -> Turbidity
 * A1 -> ORP
 * A2 -> pH
 * A3 -> Dissolved oxygen
 * A4 -> Electrical conductivity
 * 
 * Arduino and NodeMCU need a shared GND.
 * 
 * Make sure you write down the calibration offsets for pH and ORP!
 * 
 * Calibrate DisO and EC according to the documentation 
 * as they write/ read values to/ from EEPROM!
 * 
 */

#include <AltSoftSerial.h>
#include <ArduinoJson.h>
#include <avr/pgmspace.h>
#include <DFRobot_EC10.h>
#include <EEPROM.h>
#include <SoftTimer.h>

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}
#define uartInterval 2000
#define samplingInterval 40
#define ArrayLength  40
#define VOLTAGE  5.0
#define TURB_PIN A0
#define ORP_PIN A1
#define ORP_OFFSET 0 // adjust here!
#define PH_PIN A2
#define PH_OFFSET 0 // adjust here!
#define DISO_PIN A3
#define EC_PIN A4


/*
 * DisO specific
 * 
 * Saturation dissolved oxygen concentrations
 * at various temperatures
 *
 */
const float SaturationValueTab[41] PROGMEM = {      
14.46, 14.22, 13.82, 13.44, 13.09,
12.74, 12.42, 12.11, 11.81, 11.53,
11.26, 11.01, 10.77, 10.53, 10.30,
10.08, 9.86,  9.66,  9.46,  9.27,
9.08,  8.90,  8.73,  8.57,  8.41,
8.25,  8.11,  7.96,  7.82,  7.69,
7.56,  7.43,  7.30,  7.18,  7.07,
6.95,  6.84,  6.73,  6.63,  6.53,
6.41,
};
#define SaturationDisoVoltageAddress 12
#define SaturationDisoTemperatureAddress 16
float SaturationDisoVoltage, SaturationDisoTemperature;

// Function signatures
void readDisoCharacteristicValues();
double averagearray(int* arr, int number);
void packObject(JsonArray parentArray, String type, String sensor, int pin, double value);
void takeSample(Task* me);
void sendData(Task* me);

// Global variables
int turbArray[ArrayLength];
int turbArrayIndex=0;
int orpArray[ArrayLength];
int orpArrayIndex=0;
int phArray[ArrayLength];
int phArrayIndex=0;
int disoArray[ArrayLength];
int disoArrayIndex=0;
int ecArray[ArrayLength];
int ecArrayIndex=0;
DFRobot_EC10 ecHelper;
AltSoftSerial altSerial;
Task sampleTask(samplingInterval, takeSample);
Task dataTask(uartInterval, sendData);

void setup() {
  Serial.begin(9600);
  Serial.println("start");
  altSerial.begin(9600);
  altSerial.println("Hello World");
  ecHelper.begin();
  readDisoCharacteristicValues();
 
  SoftTimer.add(&sampleTask);
  SoftTimer.add(&dataTask);
}

void takeSample(Task* me) { 
  if (analogRead(TURB_PIN) > 0.00) {
    turbArray[turbArrayIndex++]= analogRead(TURB_PIN);
    if(turbArrayIndex==ArrayLength) turbArrayIndex=0;
  }

  if (analogRead(ORP_PIN) > 0.00) {
    orpArray[orpArrayIndex++]= analogRead(ORP_PIN);
    if(orpArrayIndex==ArrayLength) orpArrayIndex=0;
  }

  if (analogRead(PH_PIN) > 0.00) {
    phArray[phArrayIndex++]= analogRead(PH_PIN);
    if(phArrayIndex==ArrayLength) phArrayIndex=0;
  }

  if (analogRead(DISO_PIN) > 0.00) {
    disoArray[disoArrayIndex++]= analogRead(DISO_PIN);
    if(disoArrayIndex==ArrayLength) disoArrayIndex=0;
  }

  if (analogRead(EC_PIN) > 0.00) {
    ecArray[ecArrayIndex++]= analogRead(EC_PIN);
    if(ecArrayIndex==ArrayLength) ecArrayIndex=0;
  }
}

void sendData(Task* me) {
  StaticJsonDocument<500> doc;
  JsonArray arr = doc.createNestedArray();
  
  // turbidity in V
  double turbAverage = averagearray(turbArray, ArrayLength);
  double turb = turbAverage * (VOLTAGE / 1024.0);
  if (turb > 0.00) {
    packObject(arr, "turbidity", "SEN0189", TURB_PIN, turb);
  }
  
  // or potential in mV
  double orpAverage = averagearray(orpArray, ArrayLength); 
  double orp = ((30*(double)VOLTAGE * 1000)-(75*orpAverage*VOLTAGE*1000/1024))/75-ORP_OFFSET;
  if (orp > 0.00) {
    packObject(arr, "oxidation_reduction_potential", "SEN0165", ORP_PIN, orp);
  }

  // pH Value between 0 and 14, 7 is neutral
  double phAverage = averagearray(phArray, ArrayLength); 
  double ph = 3.5 * (phAverage * VOLTAGE /1024) + PH_OFFSET;
  if (ph > 0.00) {
    packObject(arr, "pH", "SEN0161", PH_PIN, ph);
  }

  // dissolved oxygen in mg/L
  double disoAverage = averagearray(disoArray, ArrayLength);
  float diso = pgm_read_float_near( &SaturationValueTab[0] + (int)(SaturationDisoTemperature+0.5) ) * (disoAverage * VOLTAGE / 1024) / SaturationDisoVoltage;
  if (diso > 0.00) {
    packObject(arr, "dissolved_oxygen", "SEN0237-A", DISO_PIN, diso);  
  }

  // electrical conductivity in ms/cm
  // the ec sensor needs a temperature to calcualte conductivity
  // as long as we don't have the temp sensor in the setup 
  // we just fake ideal 25^C;
  float temperature = 25.00;
  double ecAverage = averagearray(ecArray, ArrayLength);
  double ec = ecHelper.readEC(ecAverage * VOLTAGE/ 1024, temperature);
  if (ec > 0.00) {
    packObject(arr, "electrical_conductivity", "DFR0300-H", EC_PIN, ec);  
  }
  
  serializeJsonPretty(doc, Serial);
  serializeJson(doc, altSerial);
}

void packObject(JsonArray parentArray, String type, String sensor, int pin, double value) {
  JsonObject obj = parentArray.createNestedObject();
  obj["type"] = type;
  obj["sensor"] = sensor;
  obj["pin"] = pin;
  obj["value"] = value;
  return obj;
}

void readDisoCharacteristicValues() {
    EEPROM_read(SaturationDisoVoltageAddress, SaturationDisoVoltage);
    EEPROM_read(SaturationDisoTemperatureAddress, SaturationDisoTemperature);
    if(EEPROM.read(SaturationDisoVoltageAddress)==0xFF 
      && EEPROM.read(SaturationDisoVoltageAddress+1)==0xFF 
      && EEPROM.read(SaturationDisoVoltageAddress+2)==0xFF 
      && EEPROM.read(SaturationDisoVoltageAddress+3)==0xFF) {
      SaturationDisoVoltage = 1127.6;   //default voltage:1127.6mv
      EEPROM_write(SaturationDisoVoltageAddress, SaturationDisoVoltage);
    }
    if(EEPROM.read(SaturationDisoTemperatureAddress)==0xFF 
      && EEPROM.read(SaturationDisoTemperatureAddress+1)==0xFF 
      && EEPROM.read(SaturationDisoTemperatureAddress+2)==0xFF 
      && EEPROM.read(SaturationDisoTemperatureAddress+3)==0xFF) {
      SaturationDisoTemperature = 25.0;   //default temperature is 25^C
      EEPROM_write(SaturationDisoTemperatureAddress, SaturationDisoTemperature);
    }
}

double averagearray(int* arr, int number) {
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
