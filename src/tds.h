#ifndef TDS_H
#define TDS_H

#include <Arduino.h>

class TDS{
    public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    void begin(int baudrate);
    void setTemperature(float temp);
    void getTemperature();
    float getVoltageTDS();
    float getAnalogTDS();
    int getMedianNum(int bArray[], int iFilterLen);
    float getEC();
    float getTDS();
    void print(int time);

    private:
    uint8_t _pin;
    int _baudrate, _time;
    double _vref, _aref;
    float _temp;
};

#endif