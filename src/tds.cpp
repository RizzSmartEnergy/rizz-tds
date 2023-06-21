#include "TDS.h"

#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float ecval = 0;
float averageVoltage = 0;

TDS::TDS(uint8_t pin, double vref, double aref)
{
  _pin = pin;
  _vref = vref;
  _aref = aref;
}

TDS::~TDS()
{
}

void TDS::begin(int baudrate)
{
  _baudrate = baudrate;
  Serial.begin(_baudrate);
  pinMode(_pin, INPUT);
}

void TDS::setTemperature(float temp){
  _temp = temp;
}

void TDS::getTemperature(){
  return _temp;
}

float TDS::getAnalogTDS(){
  return analogRead(_pin);
}

float TDS::getVoltageTDS(){
  return (_vref * analogRead(_pin) / _aref);
}

int TDS::getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  bTemp = ((iFilterLen & 1) > 0) ? bTab[(iFilterLen - 1) / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

  return bTemp;
}

float TDS::getEC()
{
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(_pin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
    {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * _vref / _aref;
      float compensationFactor = 1.0 + 0.02 * (_temp - 25.0);
      float compensationVoltage = averageVoltage / compensationFactor;
      ecval = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
    }
  }
  return ecval;
}

float TDS::getTDS(){
  return getEC()*0.5;
}

void TDS::print(int time)
{
  _time = time;
  Serial.print("TDS Value:");
  Serial.print(getTDS(), 0);
  Serial.println("ppm");
  delay(_time);
}