#include <Arduino.h>
#include <TDS.h>

#define SENSOR_PIN 32
#define VREF 3.3    //voltage reference
#define AREF 4095   //analog range

TDS sensor(SENSOR_PIN, VREF, AREF);

void setup(){
  sensor.begin(115200); //baudrate
}

void loop(){
  sensor.print(1000); //delay time
}