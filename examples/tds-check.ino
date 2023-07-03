#include <Arduino.h>
#include <TDS.h>

#define SENSOR_PIN 32
#define VREF 3300    //voltage reference
#define AREF 4095   //analog range

TDS sensor(SENSOR_PIN, VREF, AREF);

void setup(){
  sensor.begin(); //baudrate
}

void loop(){
  sensor.getAllTDSData(); //delay time
}