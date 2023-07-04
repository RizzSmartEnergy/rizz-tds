/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
 
 ***************************************************
 This sample code shows how to read the tds value and calibrate it with the standard buffer solution.
 707ppm(1413us/cm)@25^c standard buffer solution is recommended.
 
 Created 2018-1-3
 By Jason <jason.ling@dfrobot.com@dfrobot.com>
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution.
 ****************************************************/

#ifndef TDS_H
#define TDS_H

#include "Arduino.h"

#define ReceivedBufferLength 15
#define TdsFactor 0.5  // tds = ec / 2

class TDS
{
public:
    //TDS();
    TDS(uint8_t pin, double vref, double aref);
~TDS();

    void begin();  //initialization
    void update();
    void setTemperature(float temp);  //set the temperature and execute temperature compensation
float getTemperature();
    float getKvalue();
float analogTDS();

float voltageTDS();

float compensatedVoltage();

float funcx();

float temperatureCompensation();
 
    float getTdsValue();
    float getEcValue();


    void readKValues();
    boolean cmdSerialDataAvailable();
    byte cmdParse();
    void ecCalibration(byte mode);
private:
    int _pin;
    float _vref;  // default 5.0V on Arduino UNO
    float _aref;
    float _temp;
};  

#endif