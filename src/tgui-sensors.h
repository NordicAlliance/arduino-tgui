/*!
 * @file tgui-sensors.h
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

#include <tgui-common.h>

#include "RunningMedian.h"
#include <Battery.h>
#include <VL53L0X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ODROID_Si1132.h>
#include <Zforce.h>

/* Parameters */
#define FILTER_SAMPLE_SIZE 7


enum
{
    BME280_TEMPERATURE = 0,
    BME280_HUMIDITY,
    BME280_PRESSURE,
    BME280_ALTITUDE,
};

enum
{
    VL53L0X_DISTANCE = 0,
};

enum
{
    BATTERY_LEVEL = 0,
    BATTERY_VOLTAGE,
};

enum
{
    SI1132_VISIBLE = 0,
    SI1132_IR,
    SI1132_UV,
};

enum
{
    ZFORCE_TOUCH,
    ZFORCE_X,
    ZFORCE_Y,
    ZFORCE_STATUS,
};


class SensorBME280 : public Sensor
{
private:
    uint16_t _address;
    Adafruit_BME280 _phy;
    RunningMedian _filter = RunningMedian(FILTER_SAMPLE_SIZE);
    RunningMedian _filterHumidity = RunningMedian(FILTER_SAMPLE_SIZE);
    RunningMedian _filterTemperature = RunningMedian(FILTER_SAMPLE_SIZE);
    RunningMedian _filterAltitude = RunningMedian(FILTER_SAMPLE_SIZE);
    void addDataPoint(uint8_t channel, float data);

public:
    SensorBME280(
        uint16_t i2cAddress,
        uint16_t reportInterval = 100)
    {
        _address = i2cAddress;
        _reportInterval = reportInterval;
        _filterSize = _filter.getSize() / 2 + 1;
    }
    void init();
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateTemperature();
    void updateHumidity();
    void updatePressure();
    void updateAltitude();
};

class SensorVL53L0X : public Sensor
{
private:
    uint16_t _address;
    VL53L0X _phy;
    RunningMedian _filter = RunningMedian(FILTER_SAMPLE_SIZE);
    void addDataPoint(uint8_t channel, float data);

public:
    SensorVL53L0X(
        uint16_t i2cAddress,
        uint16_t reportInterval = 100)
    {
        _address = i2cAddress;
        _reportInterval = reportInterval;
        _filterSize = _filter.getSize() / 2 + 1;
    }
    void init();
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateData();
};

class SensorSi1132 : public Sensor
{
private:
    uint16_t _address;
    ODROID_Si1132 _phy;
    RunningMedian _filterIR = RunningMedian(FILTER_SAMPLE_SIZE);
    RunningMedian _filter = RunningMedian(FILTER_SAMPLE_SIZE);
    RunningMedian _filterUV = RunningMedian(FILTER_SAMPLE_SIZE);
    void addDataPoint(uint8_t channel, float data);

public:
    SensorSi1132(
        uint16_t i2cAddress,
        uint16_t reportInterval = 100)
    {
        _address = i2cAddress;
        _reportInterval = reportInterval;
        _filterSize = _filter.getSize() / 2 + 1;
    }
    void init();
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateIR();
    void updateVisible();
    void updateUV();
};

class SensorBattery : public Sensor
{
private:
    Battery _phy = Battery(3400, 4200, A2);
    uint8_t _level;
    uint16_t _voltage;
    void addDataPoint(uint8_t channel, float data);

public:
    SensorBattery(
        uint16_t reportInterval = 100)
    {
        _reportInterval = reportInterval;
        _filterSize = 1;
    }
    void init();
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateLevel(uint8_t adjustment = 24);
    void updateVoltage();
};


typedef struct TouchPoint
{
    Location loc;
    uint8_t state;
} TouchPoint;

class Touch : public Sensor
{
private:
    Zforce _phy = Zforce();
    TouchPoint _touch;
    void addDataPoint(uint8_t channel, float data);

public:
    Touch(
        uint16_t reportInterval = 100)
    {
        _reportInterval = reportInterval;
        _filterSize = 1;
        _touch.loc.x = 0;
        _touch.loc.y = 0;
        _touch.state = 3;
    }
    void init();
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateTouch();
    TouchPoint * getLatestTouch();
    uint16_t getParameters(uint16_t input);
};
