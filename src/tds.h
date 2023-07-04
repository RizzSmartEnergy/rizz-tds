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
    TDS(uint8_t pin, double vref, double aref);
    ~TDS();
    boolean cmdSerialDataAvailable();

    byte cmdParse();

    void ecCalibration(byte mode);

    int getMedianDO(int bArray[], int iFilterLen);

    void readKValues();

    void setTemperature(float temp);

    float getTemperature();

    float analogTDS();

    float voltageTDS();

    float samplingTDS();

    float getTdsValue();

    float getEcValue();

    float getKvalue();

    float compensatedVoltage();

    float funcx();

    float temperatureCompensation();

    void update();

    void getAllTDSData();

    void begin();

    void run();

private:
    int _pin;
    double _vref, _aref;
    float _temp;
};  

#endif