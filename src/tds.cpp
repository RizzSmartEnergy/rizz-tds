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
char receivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
byte receivedBufferIndex;

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
      compensatedVoltage = averageVoltage / compensationFactor();
    }
  }
  return compensatedVoltage;
}

float TDS::compensationFactor(){
  return (1.0 + 0.02 * (_temp - 25.0));
}

float TDS::getEC25(){
  return (133.42 * samplingTDS() * samplingTDS() * samplingTDS() - 255.86 * samplingTDS() * samplingTDS() + 857.39 * samplingTDS());
}

float TDS::getEC()
{
  return getEC25() * kValue;
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
	if(serialDataTDS() > 0)
        {
          byte modeIndex = uartParsingTDS();
            calibrationEC(modeIndex);  // if received serial cmd from the serial monitor, enter into the calibration mode
        }
    EEPROM.commit();
}

boolean TDS::serialDataTDS()
{
  char receivedChar;
  static unsigned long receivedTimeOut = millis();
  while (Serial.available()>0) 
  {   
    if (millis() - receivedTimeOut > 500U) 
    {
      receivedBufferIndex = 0;
      memset(receivedBuffer,0,(ReceivedBufferLength+1));
    }
    receivedTimeOut = millis();
    receivedChar = Serial.read();
    if (receivedChar == '\n' || receivedBufferIndex==ReceivedBufferLength){
		receivedBufferIndex = 0;
		strupr(receivedBuffer);
		return true;
    }else{
      receivedBuffer[receivedBufferIndex] = receivedChar;
      receivedBufferIndex++;
    }
  }
  return false;
}

void TDS::getAllTDSData()
{
  Serial.print("Temp: " + String(getTemperature()) + " | ");
  Serial.print("K Val: " + String(kValue) + " | ");
  Serial.print("EC: " + String(getEC()) + " µS/cm | ");
  Serial.print("TDS: " + String(getTDS()) + " ppm | ");
  Serial.println("Resistivity: " + String(getResistivity()) + " kΩ.cm");
  Serial.print("Analog In: " + String(getAnalogTDS()) + " | ");
  Serial.print("Voltage In: " + String(getVoltageTDS()) + " | ");
}

byte TDS::uartParsingTDS()
{
  byte modeIndex = 0;
  modeIndex = (strstr(receivedBuffer, "ENTER") != NULL) ? 1 :
              (strstr(receivedBuffer, "CAL") != NULL) ? 2 :
              (strstr(receivedBuffer, "EXIT") != NULL) ? 3 : 0;
  return modeIndex;
}

void TDS::calibrationEC(byte mode)
{
    char *receivedBufferPtr;
    static boolean finishCalEC = 0;
    static boolean enterCalMode = 0;
    float KValueTemp,rawECsolution;
    switch(mode)
    {
      case 0:
      if(enterCalMode)
         Serial.println(F("Command Error"));
      break;
      
      case 1:
      enterCalMode = 1;
      finishCalEC = 0;
      Serial.println();
      Serial.println(F(">>>Enter Calibration Mode<<<"));
      Serial.println(F(">>>Please put the probe into the standard buffer solution<<<"));
      Serial.println();
      break;
     
      case 2:
      receivedBufferPtr=strstr(receivedBuffer, "CAL");
      receivedBufferPtr+=strlen("CAL");
      rawECsolution = strtod(receivedBufferPtr,NULL)/(float)(0.5);//TdsFactor
      rawECsolution = rawECsolution*compensationFactor();
      if(enterCalMode)
      {
          KValueTemp = rawECsolution/getEC25();  //calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
          if((rawECsolution>0) && (rawECsolution<2000) && (KValueTemp>0.25) && (KValueTemp<4.0))
          {
              Serial.println();
              Serial.print(F(">>>Confirm Successful, K:"));
              Serial.print(KValueTemp);
              Serial.println(F(", Send EXIT to Save and Exit<<<"));
              kValue =  KValueTemp;
              finishCalEC = 1;
          }
          else{
            Serial.println();
            Serial.println(F(">>>Confirm Failed, Try Again<<<"));
            Serial.println();
            finishCalEC = 0;
          }        
      }
      break;

        case 3:
        if(enterCalMode)
        {
            Serial.println();
            if(finishCalEC)
            {
               EEPROM_write(kValueAddr, kValue);
               Serial.print(F(">>>Calibration Successful, K Value Saved"));
            }
            else Serial.print(F(">>>Calibration Failed"));       
            Serial.println(F(", Exit Calibration Mode<<<"));
            Serial.println();
            finishCalEC = 0;
            enterCalMode = 0;
        }
        break;
    }
}

void TDS::kCharacteristic()
{
    EEPROM_read(kValueAddr, kValue);  
    if(EEPROM.read(kValueAddr)==0xFF && EEPROM.read(kValueAddr+1)==0xFF && EEPROM.read(kValueAddr+2)==0xFF && EEPROM.read(kValueAddr+3)==0xFF)
    {
      kValue = 1.0;
      EEPROM_write(kValueAddr, kValue);
    }
}

void TDS::run(){
  samplingTDS();
  modeTDS();
}