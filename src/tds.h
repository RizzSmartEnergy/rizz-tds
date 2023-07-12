#ifndef TDS_H
#define TDS_H

#include "Arduino.h"

class TDS
{
public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    boolean serialDataTDS();
    void stringToChar(String str, char charArray[]);
    void outputSerial2();
    byte uartParsingTDS();
    void calibrationEC(byte mode);
    int getMedianTDS(int bArray[], int iFilterLen);
    void characteristicKVal();
    void setTemperature(float temp);
    float getTemperature();
    float analogTDS();
    float voltageTDS();
    float samplingTDS();
    float compensatedVoltage();
    float ec25();
    float temperatureCompensation();
    float getKvalue();
    float getEC();
    float getTDS();
    float getResistivity();
    float getSalinity();
    void modeTDS();
    void getAllTDSData();
    void begin();
    void run();
private:
    int _pin;
    double _vref, _aref;
    float _temp;
};  

#endif