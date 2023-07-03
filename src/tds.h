#ifndef TDS_H
#define TDS_H

#include <Arduino.h>

class TDS
{
public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    void begin();
    void setTemperature(float temp);
    float getTemperature();
    float getAnalogTDS();
    float getVoltageTDS();
    int getMedianTDS(int bArray[], int iFilterLen);
    float samplingTDS();
    float getEC();
    float getTDS();
    float getResistivity();
    void getAllTDSData();
    byte uartParsingTDS();
    void calibrationEC(byte mode);
    void kCharacteristic();
    void run();
    void modeTDS();

private:
    uint8_t _pin;
    double _vref, _aref;
    float _temp;
};

#endif