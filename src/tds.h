#ifndef TDS_H
#define TDS_H

#include "Arduino.h"

class TDS
{
public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    boolean serialDataTDS();
    byte uartParsingTDS();
    boolean extInEnter(bool enterCal);
    boolean extInCal(bool exitCal);
    boolean extInExit(bool exitCal);
    //void deleteMemory(bool *enterStat);
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
    bool _enterCal, _calMode, _exitCal;
};  

#endif