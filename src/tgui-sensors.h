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

// #define USE_SI1132
// #define USE_VL53L0X
// #define USE_BME280
#define USE_BATTERY
// #define USE_ZFORCE

/* Parameters */
#define FILTER_SAMPLE_SIZE 7

//------------------------- Analog input --------------------------------/
class SensorAdc : public Sensor
{
private:
    uint8_t _pin;
    RunningMedian _filter = RunningMedian(FILTER_SAMPLE_SIZE);
    void addDataPoint(uint8_t channel, float data) { _filter.add(data); }

public:
    SensorAdc( uint8_t pin = A0, uint16_t reportInterval = 50) {
        _pin = pin;
        _reportInterval = reportInterval;
        _filterSize = 4;
    }

    void init() { pinMode(_pin, INPUT); }

    float readDataPoint(uint8_t channel = 0, bool getRawData = false) {
        if (getRawData)
            return _filter.getElement(_filter.getSize() - 1);
        else
            return _filter.getAverage(_filterSize);
    }

    void updateValue() { addDataPoint(0, analogRead(_pin)); }
};

//------------------------- Digital input -------------------------------------/
class SensorDigital : public Sensor
{
private:
    float _value;

public:
    SensorDigital() {}
    void init() {}
    float readDataPoint(uint8_t channel = 0, bool getRawData = false) { return _value; }
    void updateValue(float input) { _value = input; }
};

//------------------------- BME280 -------------------------------------/
#ifdef USE_BME280
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

enum
{
    BME280_TEMPERATURE = 0,
    BME280_HUMIDITY,
    BME280_PRESSURE,
    BME280_ALTITUDE,
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
#endif

//------------------------ VL53L0X ---------------------------------------/
#ifdef USE_VL53L0X
#include <VL53L0X.h>

enum
{
    VL53L0X_DISTANCE = 0,
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
#endif

//------------------------ Si1132 ---------------------------------------/
#ifdef USE_SI1132
#include <ODROID_Si1132.h>

enum
{
    SI1132_VISIBLE = 0,
    SI1132_IR,
    SI1132_UV,
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
#endif

//------------------------ Battery ---------------------------------------/
#ifdef USE_BATTERY
#include <Battery.h>

enum
{
    BATTERY_LEVEL = 0,
    BATTERY_VOLTAGE,
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
#endif

//------------------------ Zforce touch ---------------------------------------/
#ifdef USE_ZFORCE
#include <Zforce.h>

typedef struct TouchPoint
{
    Location loc;
    uint8_t state;
} TouchPoint;

enum
{
    ZFORCE_TOUCH,
    ZFORCE_X,
    ZFORCE_Y,
    ZFORCE_STATUS,
};

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
    TouchPoint *getLatestTouch();
    uint16_t getParameters(uint16_t input);
};
#endif
