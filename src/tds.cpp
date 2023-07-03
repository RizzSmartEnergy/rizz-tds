#include "TDS.h"

#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float compensatedVoltage = 0;
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

void TDS::begin()
{
  pinMode(_pin, INPUT);
}

void TDS::setTemperature(float temp)
{
  _temp = temp;
}

float TDS::getTemperature()
{
  return _temp;
}

float TDS::getAnalogTDS()
{
  return analogRead(_pin);
}

float TDS::getVoltageTDS()
{
  return (analogRead(_pin) * _vref / (1000 * _aref));
}

int TDS::getMedianTDS(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
  {
    bTab[i] = bArray[i];
  }
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

float TDS::samplingTDS()
{
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 30U)
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = getAnalogTDS();
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
      averageVoltage = getMedianTDS(analogBufferTemp, SCOUNT) * _vref / (1000 * _aref);
      float compensationFactor = 1.0 + 0.02 * (_temp - 25.0);
      compensatedVoltage = averageVoltage / compensationFactor;
    }
  }
  return compensatedVoltage;
}

float TDS::getEC()
{
  return (133.42 * samplingTDS() * samplingTDS() * samplingTDS() - 255.86 * samplingTDS() * samplingTDS() + 857.39 * samplingTDS()) * 1.0;
}

float TDS::getTDS()
{
  return getEC() * 0.64;
}

float TDS::getResistivity()
{
  return getEC() == 0 ? 0 : 1000 / getEC();
}

void TDS::getAllTDSData()
{
  Serial.print("Temperature: " + String(getTemperature()) + " | ");
  Serial.print("Input Analog: " + String(getAnalogTDS()) + " | ");
  Serial.print("Input Voltage: " + String(getVoltageTDS()) + " | ");
  Serial.print("EC: " + String(getEC()) + " µS/cm | ");
  Serial.print("TDS: " + String(getTDS()) + " ppm | ");
  Serial.println("Resistivity: " + String(getResistivity()) + " kΩ.cm");
}
