#include <EEPROM.h>
#include "TDS.h"

#define EEPROM_write(address, p)        \
  {                                     \
    int i = 0;                          \
    byte *pp = (byte *)&(p);            \
    for (; i < sizeof(p); i++)          \
      EEPROM.write(address + i, pp[i]); \
  }
#define EEPROM_read(address, p)         \
  {                                     \
    int i = 0;                          \
    byte *pp = (byte *)&(p);            \
    for (; i < sizeof(p); i++)          \
      pp[i] = EEPROM.read(address + i); \
  }

#define ReceivedBufferLength 20
char receivedBuffer[ReceivedBufferLength + 1];
byte receivedBufferIndex = 0;

#define ReceivedBufferLength2 20
char receivedBuffer2[ReceivedBufferLength2 + 1];
byte receivedBufferIndex2 = 0;

#define SCOUNT 30
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;

#define TDSFactor 0.64
#define kValAddr 8
float kVal;
float avgVolt;

String receivedString2;

TDS::TDS(uint8_t pin, double vref, double aref)
{
  _pin = pin;
  _vref = vref;
  _aref = aref;
}

TDS::~TDS()
{
}

boolean TDS::serialDataTDS()
{
  char receivedChar;
  static unsigned long receivedTimeOut = millis();
  while (Serial.available() > 0)
  {
    if (millis() - receivedTimeOut > 500U)
    {
      receivedBufferIndex = 0;
      memset(receivedBuffer, 0, (ReceivedBufferLength + 1));
    }
    receivedTimeOut = millis();
    receivedChar = Serial.read();
    if (receivedChar == '\n' || receivedBufferIndex == ReceivedBufferLength)
    {
      receivedBufferIndex = 0;
      strupr(receivedBuffer);
      return true;
    }
    else
    {
      receivedBuffer[receivedBufferIndex] = receivedChar;
      receivedBufferIndex++;
    }
  }
  return false;
}

void TDS::stringToChar(String str, char charArray[])
{
  int length = str.length();
  for (int i = 0; i < length; i++)
  {
    charArray[i] = str.charAt(i);
  }
  charArray[length] = '\n';
}

  
void TDS::outputSerial2()
{
  // String receivedString2 = Serial2.readString();
  // stringToChar(receivedString2, receivedBuffer2);
  // Serial.println(receivedBuffer2);
  String receivedString2="";
  if (Serial2.available())
  {
    delay(30);
    while (Serial2.available())
    {
      receivedString2 += char(Serial2.read());
      // Serial2.write(0xff);
      // Serial2.write(0xff);
      // Serial2.write(0xff);
    }
      Serial.println(receivedString2);
  }
}

byte TDS::uartParsingTDS()
{
  byte modeIndex = 0;
  if (strstr(receivedBuffer, "ENTER") != NULL || strstr(receivedBuffer2, "ENTER") != NULL)
  {
    modeIndex = 1;
  }
  else if (strstr(receivedBuffer, "CAL:") != NULL || strstr(receivedBuffer2, "CAL:") != NULL)
  {
    modeIndex = 2;
  }
  else if (strstr(receivedBuffer, "EXIT") != NULL || strstr(receivedBuffer2, "EXIT") != NULL)
  {
    modeIndex = 3;
  }
  return modeIndex;
}

void TDS::calibrationEC(byte mode)
{
  char *receivedBufferPtr;
  static boolean finishCalEC = 0, enterCalMode = 0;
  float KValueTemp, rawECsolution;
  switch (mode)
  {
  case 0:
    if (enterCalMode)
      Serial.println(F("Command Error"));
    break;

  case 1:
    enterCalMode = 1;
    finishCalEC = 0;
    Serial.println();
    Serial.println(F(">>>Enter Calibration Mode<<<"));
    Serial.println(F(">>>Please put the probe into the standard buffer solution!<<<"));
    Serial.println();
    break;

  case 2:
    receivedBufferPtr = strstr(receivedBuffer2, "CAL:");
    receivedBufferPtr += strlen("CAL:");
    rawECsolution = strtod(receivedBufferPtr, NULL) / (float)(TDSFactor);
    rawECsolution = rawECsolution * temperatureCompensation();
    if (enterCalMode)
    {
      KValueTemp = rawECsolution / compensatedVoltage();
      if ((rawECsolution > 0) && (rawECsolution < 2000) && (KValueTemp > 0.25) && (KValueTemp < 4.0))
      {
        Serial.println();
        Serial.print(F(">>>Confirm Successful, K:"));
        Serial.print(KValueTemp);
        Serial.println(F(", Send EXIT to Save and Exit<<<"));
        kVal = KValueTemp;
        finishCalEC = 1;
      }
      else
      {
        Serial.println();
        Serial.println(F(">>>Confirm Failed,Try Again<<<"));
        Serial.println();
        finishCalEC = 0;
      }
    }
    break;

  case 3:
    if (enterCalMode)
    {
      Serial.println();
      if (finishCalEC)
      {
        EEPROM_write(kValAddr, kVal);
        Serial.print(F(">>>Calibration Successful, K Value Saved"));
      }
      else
        Serial.print(F(">>>Calibration Failed"));
      Serial.println(F(", Exit Calibration Mode<<<"));
      Serial.println();
      finishCalEC = 0;
      enterCalMode = 0;
    }
    break;
  }
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

void TDS::characteristicKVal()
{
  EEPROM_read(kValAddr, kVal);
  if (EEPROM.read(kValAddr) == 0xFF && EEPROM.read(kValAddr + 1) == 0xFF && EEPROM.read(kValAddr + 2) == 0xFF && EEPROM.read(kValAddr + 3) == 0xFF)
  {
    kVal = 1.0; // default value: K = 1.0
    EEPROM_write(kValAddr, kVal);
  }
}

void TDS::setTemperature(float temp)
{
  _temp = temp;
}

float TDS::getTemperature()
{
  return _temp;
}

float TDS::analogTDS()
{
  return analogRead(_pin);
}

float TDS::voltageTDS()
{
  return (analogTDS() * _vref) / _aref;
}

float TDS::samplingTDS()
{
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogTDS();
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    avgVolt = getMedianTDS(analogBufferTemp, SCOUNT) * _vref / (1000 * _aref);
  }
  return avgVolt;
}

float TDS::compensatedVoltage()
{
  // basic equation
  // return (133.42 * voltageTDS() * voltageTDS() * voltageTDS() - 255.86 * voltageTDS() * voltageTDS() + 857.39 * voltageTDS());
  // more stable
  return (133.42 * samplingTDS() * samplingTDS() * samplingTDS() - 255.86 * samplingTDS() * samplingTDS() + 857.39 * samplingTDS());
}

float TDS::ec25()
{
  return compensatedVoltage() * kVal;
}

float TDS::temperatureCompensation()
{
  return (1.0 + 0.02 * (_temp - 25.0));
}

float TDS::getKvalue()
{
  return kVal;
}

float TDS::getEC()
{
  return ec25() / temperatureCompensation();
}

float TDS::getTDS()
{
  return getEC() * TDSFactor;
}

float TDS::getResistivity()
{
  return (getEC() == 0) ? 0 : 1000 / getEC();
}

float TDS::getSalinity()
{
  return 0.4665 * (pow((getEC() / 1000), 1.0878));
}

void TDS::modeTDS()
{
  if (serialDataTDS() > 0 || receivedBuffer2 != NULL)
  {
    byte modeIndex = uartParsingTDS();
    calibrationEC(modeIndex);
  }
  EEPROM.commit();
}

void TDS::getAllTDSData()
{
  Serial.print("Temp: " + String(getTemperature()) + "°C | ");
  Serial.print("K val: " + String(getKvalue()) + " | ");
  Serial.print("EC: " + String(getEC()) + " µS/cm | ");
  Serial.print("TDS: " + String(getTDS()) + " ppm | ");
  Serial.print("Resistivity: " + String(getResistivity()) + " kΩ.cm | ");
  Serial.print("Salinity: " + String(getSalinity()) + " psu | ");
  Serial.print("Avg Volt: " + String(samplingTDS()) + " mV | ");
  Serial.print("Volt in: " + String(voltageTDS()) + " mV | ");
  Serial.println("Analog in: " + String(analogTDS()) + " | ");
}

void TDS::begin()
{
  EEPROM.begin(512);
  pinMode(_pin, INPUT);
  characteristicKVal();
}

void TDS::run()
{
  samplingTDS();
  modeTDS();
  outputSerial2();
}
