#include "TDS.h"
#include <EEPROM.h>

#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float compensatedVoltage = 0;
float averageVoltage = 0;
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}
#define kValueAddr 8
#define ReceivedBufferLength 20
char cmdReceivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
byte cmdReceivedBufferIndex;

float kValue;

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
  EEPROM.begin(4095);
  pinMode(_pin, INPUT);
  kCharacteristic();
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

void TDS::modeTDS(){

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

byte TDS::uartParsingTDS()
{
  byte modeIndex = 0;
  if(strstr(cmdReceivedBuffer, "ENTER") != NULL) 
      modeIndex = 1;
  else if(strstr(cmdReceivedBuffer, "EXIT") != NULL) 
      modeIndex = 3;
  else if(strstr(cmdReceivedBuffer, "CAL") != NULL)   
      modeIndex = 2;
  return modeIndex;
}

void TDS::calibrationEC(byte mode)
{
    char *cmdReceivedBufferPtr;
    static boolean ecCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    float KValueTemp,rawECsolution;
    switch(mode)
    {
      case 0:
      if(enterCalibrationFlag)
         Serial.println(F("Command Error"));
      break;
      
      case 1:
      enterCalibrationFlag = 1;
      ecCalibrationFinish = 0;
      Serial.println();
      Serial.println(F(">>>Enter Calibration Mode<<<"));
      Serial.println(F(">>>Please put the probe into the standard buffer solution<<<"));
      Serial.println();
      break;
     
      case 2:
      cmdReceivedBufferPtr=strstr(cmdReceivedBuffer, "CAL");
      cmdReceivedBufferPtr+=strlen("CAL");
      rawECsolution = strtod(cmdReceivedBufferPtr,NULL)/(float)(0.5);//TdsFactor
      rawECsolution = rawECsolution*(1.0+0.02*(getTemperature()-25.0));
      if(enterCalibrationFlag)
      {
         // Serial.print("rawECsolution:");
         // Serial.print(rawECsolution);
         // Serial.print("  ecvalue:");
         // Serial.println(ecValue);
          KValueTemp = rawECsolution/(133.42*samplingTDS()*samplingTDS()*samplingTDS() - 255.86*samplingTDS()*samplingTDS() + 857.39*samplingTDS());  //calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
          if((rawECsolution>0) && (rawECsolution<2000) && (KValueTemp>0.25) && (KValueTemp<4.0))
          {
              Serial.println();
              Serial.print(F(">>>Confrim Successful,K:"));
              Serial.print(KValueTemp);
              Serial.println(F(", Send EXIT to Save and Exit<<<"));
              kValue =  KValueTemp;
              ecCalibrationFinish = 1;
          }
          else{
            Serial.println();
            Serial.println(F(">>>Confirm Failed,Try Again<<<"));
            Serial.println();
            ecCalibrationFinish = 0;
          }        
      }
      break;

        case 3:
        if(enterCalibrationFlag)
        {
            Serial.println();
            if(ecCalibrationFinish)
            {
               EEPROM_write(kValueAddr, kValue);
               Serial.print(F(">>>Calibration Successful,K Value Saved"));
            }
            else Serial.print(F(">>>Calibration Failed"));       
            Serial.println(F(",Exit Calibration Mode<<<"));
            Serial.println();
            ecCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}

void TDS::kCharacteristic()
{
    EEPROM_read(kValueAddr, kValue);  
    if(EEPROM.read(kValueAddr)==0xFF && EEPROM.read(kValueAddr+1)==0xFF && EEPROM.read(kValueAddr+2)==0xFF && EEPROM.read(kValueAddr+3)==0xFF)
    {
      kValue = 1.0;   // default value: K = 1.0
      EEPROM_write(kValueAddr, kValue);
    }

    EEPROM.commit();
}

void TDS::run(){
  samplingTDS();
}