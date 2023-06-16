#ifndef tds_H
#define tds_H

#include <Arduino.h>

class tds{
    public:
    tds(uint8_t pin, double vref, double aref);
    void begin(int baudrate);
    int getMedianNum(int bArray[], int iFilterLen);
    float tdsValue();
    void print(int time);

    private:
    uint8_t _pin;
    int _baudrate, _time;
    double _vref, _aref;
};

#endif