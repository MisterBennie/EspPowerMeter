#include <Arduino.h>
#include <WiFi.h>

#include "PowerSampler.h"
int subtract;
int _numberOfSamples = 200;
int _voltage = 230; // Assumed Voltage
int _pin = 32;
int _pins[8] = {32, 33, 34, 35, 36, 39, -1, -1};

PowerSampler* powerSampler;

void connectToNetwork() {
  WiFi.begin("OpenWrt", "zaq1xsw2");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }
 
  Serial.println("------------------------------");
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  Serial.println("------------------------------");
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //connectToNetwork();

  powerSampler = new PowerSampler(_pin, _numberOfSamples, _voltage);
  powerSampler->Start();

  // Give everything time to settle
  delay(1000);

}

void waitForInput(){
  while (Serial.read() == -1){
  }
}

void loop() {
  int actualSamples;

  int buffer;
  int averagingOverNumberOfPeriods = 50;
  long totalSubtract = 0;

  float wattage;
  float totalWattage = 0;

  int count = 0;
  int emptyCount = 0;
  int lastWattage = 0;
  
  for (int i = 0 ; i < averagingOverNumberOfPeriods ; i++ ){
    while (!powerSampler->GetActiveBuffer(&subtract, &wattage, &actualSamples, &buffer)){};
    if (actualSamples != 0){
      lastWattage = wattage;
      totalWattage += wattage;
      totalSubtract += subtract;
      count++;
    }
    else
    {
      emptyCount++;
    }
  }

  if (count != 0){
    Serial.print(abs(lastWattage),0);
    Serial.print(" : ");
    Serial.print(abs(totalWattage/count),0);
    Serial.print(" :     ");
    Serial.print(abs(abs(totalWattage/count) - abs(lastWattage)),0);
    Serial.print("     : ");
    Serial.print(totalSubtract/count);
    Serial.print(" : ");
    Serial.print(count);
    Serial.print(" : ");
    Serial.print(actualSamples);
    Serial.print(" : ");
    Serial.println(emptyCount);
  }
}