#ifndef TDS_H
#define TDS_H

#include <Arduino.h>

class TDS
{
public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    void begin(int baudrate);
    void setTemperature(float temp);
    float getTemperature();
    float getAnalogTDS();
    float getVoltageTDS();
    int getMedianTDS(int bArray[], int iFilterLen);
    float samplingTDS();
    float getEC();
    float getTDS();
    float getResistivity();
    void getAllTDSData(int delay_time);

private:
    uint8_t _pin;
    double _vref, _aref;
    float _temp;
    int _baudrate, _delay_time;
};

#endif