/*!
 * @file tgui-sensors.cpp
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

#include "tgui-sensors.h"

/* Sensor specific parameters */
#define SEALEVELPRESSURE_HPA (1013.25)
#define DATA_READY 17 //PD2(INT0) on Odroid

//------------------------- BME280 -------------------------------------/
void SensorBME280::init()
{
    bool status = _phy.begin(_address);
    if (!status)
        Sprintln(F("No BME280 sensor"));
}

void SensorBME280::addDataPoint(uint8_t channel, float data)
{
    RunningMedian *filter;
    switch (channel)
    {
    case BME280_TEMPERATURE:
        filter = &_filterTemperature;
        break;
    case BME280_HUMIDITY:
        filter = &_filterHumidity;
        break;
    case BME280_PRESSURE:
        filter = &_filter;
        break;
    case BME280_ALTITUDE:
        filter = &_filterAltitude;
        break;

    default:
        filter = &_filter;
        break;
    }
    filter->add(data);
}

float SensorBME280::readDataPoint(uint8_t channel, bool getRawData)
{
    RunningMedian *filter;
    switch (channel)
    {
    case BME280_TEMPERATURE:
        filter = &_filterTemperature;
        break;
    case BME280_HUMIDITY:
        filter = &_filterHumidity;
        break;
    case BME280_PRESSURE:
        filter = &_filter;
        break;
    case BME280_ALTITUDE:
        filter = &_filterAltitude;
        break;

    default:
        filter = &_filter;
        break;
    }
    
    if (getRawData)
    {
        return filter->getElement(filter->getSize() - 1);
    }
    else
    {
        return filter->getAverage(_filterSize);
    }
}

void SensorBME280::updateTemperature()
{
    addDataPoint(BME280_TEMPERATURE, _phy.readTemperature());
}

void SensorBME280::updateHumidity()
{
    addDataPoint(BME280_HUMIDITY, _phy.readHumidity());
}

void SensorBME280::updatePressure()
{
    // usually the pressure stays between 980 and 1030hpa
    // Record in Sweden shows the upper and lower bounds are 938.4 and 1063.7hpa
    addDataPoint(BME280_PRESSURE, (_phy.readPressure()-98000.0)/1000.0);
}

void SensorBME280::updateAltitude()
{
    addDataPoint(BME280_ALTITUDE, _phy.readAltitude(SEALEVELPRESSURE_HPA));
}

//------------------------ VL53L0X ---------------------------------------/
#define LONG_RANGE
//#define HIGH_SPEED
#define HIGH_ACCURACY

void SensorVL53L0X::init()
{
    _phy.init();
    _phy.setTimeout(200);

#if defined LONG_RANGE
    // lower the return signal rate limit (default is 0.25 MCPS)
    _phy.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    _phy.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    _phy.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif
#if defined HIGH_SPEED
    // reduce timing budget to 20 ms (default is about 33 ms)
    _phy.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
    // increase timing budget to 100 ms
    _phy.setMeasurementTimingBudget(100000);
#endif
    Sprintln("VL53L0X initialized");
}

void SensorVL53L0X::addDataPoint(uint8_t channel, float data)
{
    RunningMedian *filter = &_filter;
    filter->add(data);
}

float SensorVL53L0X::readDataPoint(uint8_t channel = 0, bool getRawData = false)
{
    RunningMedian *filter = &_filter;

    if (getRawData)
    {
        return filter->getElement(filter->getSize() - 1);
    }
    else
    {
        return filter->getAverage(_filterSize);
    }
}

void SensorVL53L0X::updateData()
{
    addDataPoint(0, _phy.readRangeSingleMillimeters());
}

//------------------------ Si1132 ---------------------------------------/
void SensorSi1132::init()
{
    _phy.begin();
}

void SensorSi1132::addDataPoint(uint8_t channel, float data)
{
    RunningMedian *filter;
    switch (channel)
    {
    case SI1132_VISIBLE:
        filter = &_filter;
        break;
    case SI1132_IR:
        filter = &_filterIR;
        break;
    case SI1132_UV:
        filter = &_filterUV;
        break;

    default:
        filter = &_filter;
        break;
    }
    filter->add(data);
}

float SensorSi1132::readDataPoint(uint8_t channel, bool getRawData)
{
    RunningMedian *filter;
    switch (channel)
    {
    case SI1132_VISIBLE:
        filter = &_filter;
        break;
    case SI1132_IR:
        filter = &_filterIR;
        break;
    case SI1132_UV:
        filter = &_filterUV;
        break;

    default:
        filter = &_filter;
        break;
    }

    if (getRawData)
    {
        return filter->getElement(filter->getSize() - 1);
    }
    else
    {
        return filter->getAverage(_filterSize);
    }
}

void SensorSi1132::updateIR()
{
    addDataPoint(SI1132_IR, _phy.readIR());
}

void SensorSi1132::updateVisible()
{
    addDataPoint(SI1132_VISIBLE, _phy.readVisible());
}

void SensorSi1132::updateUV()
{
    addDataPoint(SI1132_UV, _phy.readUV());
}

//------------------------ Battery ---------------------------------------/
void SensorBattery::init()
{
    _phy.begin(1094, 4.076923, &sigmoidal);
}

void SensorBattery::addDataPoint(uint8_t channel, float data)
{
    switch (channel)
    {
    case BATTERY_LEVEL:
        _level = data;
        break;
    case BATTERY_VOLTAGE:
        _voltage = data;
        break;

    default:
        break;
    }
}

float SensorBattery::readDataPoint(uint8_t channel = 0, bool getRawData = false)
{
    switch (channel)
    {
    case BATTERY_LEVEL:
        return _level;
    case BATTERY_VOLTAGE:
        return _voltage;

    default:
        return _level;
    }
}

void SensorBattery::updateVoltage()
{
    addDataPoint(BATTERY_VOLTAGE, _phy.voltage());
}

void SensorBattery::updateLevel()
{
    addDataPoint(BATTERY_LEVEL, _phy.level() + 24); // so that 75%-100% is shown as full power
}

//------------------------ Zforce touch ---------------------------------------/
void Touch::init()
{
    _phy.Start(DATA_READY);

    Message *msg = _phy.GetMessage();
    if (msg != NULL)
    {
        // Sprintln(F("zForce touch sensor detected"));
    }
    _phy.DestroyMessage(msg);
    
    _phy.ReverseX(true); // Send and read ReverseX
    do
    {
        msg = _phy.GetMessage();
    } while (msg == NULL);

    if (msg->type == MessageType::REVERSEXTYPE)
    {
        // Sprintln(F("zForce touch sensor reverse X axis"));
    }
    _phy.DestroyMessage(msg);

    uint8_t changeFreq[] = {0xEE, 0x0B, 0xEE, 0x09, 0x40, 0x02, 0x02, 0x00, 0x68, 0x03, 0x80, 0x01, 0x32};
    _phy.Write(changeFreq);
    do
    {
        msg = _phy.GetMessage();
    } while (msg == NULL);
    _phy.DestroyMessage(msg);
    // Sprintln(F("Changed frequency"));

    _phy.Enable(true);  // Send and read Enable
    do
    {
        msg = _phy.GetMessage();
    } while (msg == NULL);

    if (msg->type == MessageType::ENABLETYPE)
    {
        Sprintln(F("zForce touch sensor is ready"));
    }
    _phy.DestroyMessage(msg);
}

void Touch::addDataPoint(uint8_t channel, float data)
{
    Message* touch = _phy.GetMessage();
    if(touch != NULL)
    {
        if(touch->type == MessageType::TOUCHTYPE)
        {
            _touch.loc.x = ((TouchMessage*)touch)->touchData[0].x;
            _touch.loc.y = ((TouchMessage*)touch)->touchData[0].y;
            _touch.state = ((TouchMessage*)touch)->touchData[0].event;
            // Serial.print("X/Y/Event: ");
            // Sprint(_touch.x);
            // Serial.print(" / ");
            // Sprint(_touch.y);
            // Serial.print(" / ");
            // Sprintln(_touch.state);
        }
    }
    _phy.DestroyMessage(touch);
}

float Touch::readDataPoint(uint8_t channel = 0, bool getRawData = false)
{
    switch (channel)
    {
    case ZFORCE_X:
        return _touch.loc.x;
    case ZFORCE_Y:
        return _touch.loc.y;

    default:
        return _touch.state;
    }
}

TouchPoint * Touch::getLatestTouch(void)
{
    return &_touch;
}

uint16_t Touch::getParameters(uint16_t input)
{
    switch (_touch.state)
    {
    case DOWN:
        return 0;
    case MOVE:
        return 0;
    default:
        return 1;
    }
}

void Touch::updateTouch()
{
    addDataPoint(ZFORCE_TOUCH, 0);
}
