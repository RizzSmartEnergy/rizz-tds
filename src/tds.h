#ifndef TDS_H
#define TDS_H

#include "Arduino.h"

class TDS
{
public:
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    boolean serialDataTDS();
   // boolean serial2DataTDS();
    void convertStringToChar(const String &str, char *charArray, int maxLength);
    void outputSerial2();
    byte uartParsingTDS();
    String extInEnter(String enterCal);
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
    String _enterCal, _calMode, _exitCal;
};  

#endif