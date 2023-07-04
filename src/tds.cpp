#include <EEPROM.h>
#include "TDS.h"

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

#define kValAddr 8
float kVal;


   /the address of the K value stored in the EEPROM
    char cmdReceivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
    byte cmdReceivedBufferIndex;

TDS::TDS(uint8_t pin, double vref, double aref)
{
    _pin = pin;
    _vref = vref;
    _aref = aref;
}

TDS::~TDS()
{
}

// void TDS::setPin(int pin)
// {
// 	_pin = pin;
// }

void TDS::setTemperature(float temp)
{
	_temp = temp;
}

float TDS::getTemperature(){
  return _temp;
}

// void TDS::setVref(float vref)
// {
// 	_vref = vref;
// }

// void TDS::setAdcRange(float aref)
// {
//       _aref = aref;
// }

// void TDS::setKvalueAddress(int address)
// {
//       kValAddr = address;
// }

void TDS::begin()
{
	pinMode(_pin,INPUT);
	readKValues();
}

float TDS::getKvalue()
{
	return kVal;
}

float TDS::analogTDS(){
  return analogRead(_pin);
}

float TDS::voltageTDS(){
  return analogTDS() * _vref / _aref;
}

float TDS::compensatedVoltage(){
  return (133.42*voltageTDS()*voltageTDS()*voltageTDS() - 255.86*voltageTDS()*voltageTDS() + 857.39*voltageTDS());
}

float TDS::funcx(){
  return compensatedVoltage()*kVal;
}

float TDS::temperatureCompensation(){
  return (1.0+0.02*(_temp-25.0));
}

void TDS::update()
{
	if(cmdSerialDataAvailable() > 0)
        {
            ecCalibration(cmdParse());  // if received serial cmd from the serial monitor, enter into the calibration mode
        }
}

float TDS::getTdsValue()
{
	return getEcValue() * TdsFactor;
}

float TDS::getEcValue()
{
      return funcx() / temperatureCompensation();
}


void TDS::readKValues()
{
    EEPROM_read(kValAddr, kVal);  
    if(EEPROM.read(kValAddr)==0xFF && EEPROM.read(kValAddr+1)==0xFF && EEPROM.read(kValAddr+2)==0xFF && EEPROM.read(kValAddr+3)==0xFF)
    {
      kVal = 1.0;   // default value: K = 1.0
      EEPROM_write(kValAddr, kVal);
    }
}

boolean TDS::cmdSerialDataAvailable()
{
  char cmdReceivedChar;
  static unsigned long cmdReceivedTimeOut = millis();
  while (Serial.available()>0) 
  {   
    if (millis() - cmdReceivedTimeOut > 500U) 
    {
      cmdReceivedBufferIndex = 0;
      memset(cmdReceivedBuffer,0,(ReceivedBufferLength+1));
    }
    cmdReceivedTimeOut = millis();
    cmdReceivedChar = Serial.read();
    if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex==ReceivedBufferLength){
		cmdReceivedBufferIndex = 0;
		strupr(cmdReceivedBuffer);
		return true;
    }else{
      cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
      cmdReceivedBufferIndex++;
    }
  }
  return false;
}

byte TDS::cmdParse()
{
  byte modeIndex = 0;
  if(strstr(cmdReceivedBuffer, "ENTER") != NULL) 
      modeIndex = 1;
  else if(strstr(cmdReceivedBuffer, "EXIT") != NULL) 
      modeIndex = 3;
  else if(strstr(cmdReceivedBuffer, "CAL:") != NULL)   
      modeIndex = 2;
  return modeIndex;
}

void TDS::ecCalibration(byte mode)
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
      cmdReceivedBufferPtr=strstr(cmdReceivedBuffer, "CAL:");
      cmdReceivedBufferPtr+=strlen("CAL:");
      rawECsolution = strtod(cmdReceivedBufferPtr,NULL)/(float)(TdsFactor);
      rawECsolution = rawECsolution*temperatureCompensation();
      if(enterCalibrationFlag)
      {
         // Serial.print("rawECsolution:");
         // Serial.print(rawECsolution);
         // Serial.print("  ecvalue:");
         // Serial.println(ecValue);
          KValueTemp = rawECsolution/compensatedVoltage();  //calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
          if((rawECsolution>0) && (rawECsolution<2000) && (KValueTemp>0.25) && (KValueTemp<4.0))
          {
              Serial.println();
              Serial.print(F(">>>Confrim Successful,K:"));
              Serial.print(KValueTemp);
              Serial.println(F(", Send EXIT to Save and Exit<<<"));
              kVal =  KValueTemp;
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
               EEPROM_write(kValAddr, kVal);
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