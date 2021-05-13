#include <Arduino.h>
#include <WiFi.h> 

#include "PowerSampler.h"
int subtract;
int _numberOfSamples = 300;
int _voltage = 230; // Assumed Voltage
int _pin = 32;
int _numberOfPins; // = sizeof(_pins)/sizeof(_pins[0]);
int _pins[] = { 32, 33, 34, 35, 36, 25, 26, 27 }; // 39 does not work
//int _pins[] = { 27, 32 }; // 39 does not work
int **_bufferSamples;
float *_wattages;

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

  delay(1000);

  _numberOfPins = sizeof(_pins)/sizeof(_pins[0]);
  Serial.println(sizeof(_pins));
  Serial.println(sizeof(_pins[0]));
  Serial.println(_numberOfPins);
  Serial.println();

  //connectToNetwork();

  //  powerSampler = new PowerSampler(_pin, _numberOfSamples, _voltage);

  _bufferSamples = new int *[_numberOfPins];
  for(int i = 0; i <_numberOfPins; i++){
    _bufferSamples[i] = new int[_numberOfSamples];
  }
  _wattages = new float[_numberOfPins];

  powerSampler = new PowerSampler(_pins, _numberOfPins, _bufferSamples, _numberOfSamples, _voltage);
  powerSampler->Start();

  // Give everything time to settle
  delay(1000);

    for (int i = 0 ; i < _numberOfPins ; i++){
      Serial.print("   Pin: ");
      Serial.print(_pins[i]);
    }
    Serial.println();
}

void waitForInput(){
  while (Serial.read() == -1){
  }
}

void loop() {
  float wattage;
  int previousIndex = 0;
  int activeIndex = 0;

  // iterate over all pins and wait for 500 mSec
  for (int i = 0 ; i < _numberOfPins ; i++){
    while (!powerSampler->GetActiveBuffer(&wattage, &previousIndex, &activeIndex)){};
    _wattages[previousIndex] = wattage;
    int wattage = abs((int)_wattages[i]);
    if (wattage < 1000) Serial.print(" ");
    if (wattage < 100) Serial.print(" ");
    if (wattage < 10) Serial.print(" ");
    Serial.print(wattage);
    Serial.print(" : ");
  }
  Serial.print(_numberOfPins);
  Serial.println("                                                  \r");
  delay(500);
}