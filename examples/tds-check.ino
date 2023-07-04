#include <Arduino.h>
#include <TDS.h>

#define SENSOR_PIN 32
#define VREF 3300
#define AREF 4095

TDS sensor(SENSOR_PIN, VREF, AREF);

void setup(){
  Serial.begin(115200);
  sensor.begin();
}

void loop(){
  sensor.setTemperature(25.0);
  sensor.run();
  sensor.getAllTDSData();
  delay(1000);
}