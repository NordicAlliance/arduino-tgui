/*!
 * @file tgui.h
 *
 * Written by Wyng AB Sweden, visit us http://www.wyngstudio.com
 *
 * Apache license.
 *
 */

#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>
#include <Battery.h>
#include "RunningMedian.h"

#include <VL53L0X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ODROID_Si1132.h>

#define DEBUG
#ifdef DEBUG
#define Sprintln(a) Serial.println(a)
#define Sprint(a) Serial.print(a)
#else
#define Sprintln(a)
#define Sprint(a)
#endif

/* COMMANDS */
#define pbar_show_border

/* Parameters */
#define foregroundColor ILI9340_YELLOW
#define backgroundColor 0x0016//ILI9340_BLUE
#define defaultRotation 1
#define defaultTextSize 2
#define textPixelW(x) (6 * (x))
#define textPixelH(x) (8 * (x))
#define pbar_text_width (7 * textPixelW(defaultTextSize))
#define screenWidth 320
#define screenHeight 240
#define screenPadding 10
#define borderPadding 2
#define widgetStart (pbar_text_width + screenPadding)
#define widgetWidth (screenWidth - widgetStart - screenPadding)
#define FILTER_SAMPLE_SIZE 7
#define SEALEVELPRESSURE_HPA (1013.25)

/* REGISTERS */
#define _cs 10
#define _dc 9
#define _rst 8

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

typedef struct Location
{
    uint16_t x;
    uint16_t y ;
} Location;

typedef struct Size
{
    uint16_t width;
    uint16_t height;
} Size;

void InitializeScreen();

class Sensor
{
    protected:
        uint8_t _filterSize;

    public:
        Sensor(){};
        ~Sensor(){};
        virtual void addDataPoint(uint8_t channel, float data){};
        virtual float readDataPoint(uint8_t channel, bool getRawData) { return 0; };
        virtual void init(){};
        uint16_t _reportInterval;
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
    void addDataPoint(uint8_t channel, float data);
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
    void addDataPoint(uint8_t channel, float data);
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
    void addDataPoint(uint8_t channel, float data);
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

public:
    SensorBattery(
        uint16_t reportInterval = 100)
    {
        _reportInterval = reportInterval;
        _filterSize = 1;
    }
    void init();
    void addDataPoint(uint8_t channel, float data);
    float readDataPoint(uint8_t channel, bool getRawData);
    void updateLevel();
    void updateVoltage();
};

class TguiElement
{
    protected:
        /* data */
        Location _loc;
        Size _size;
        uint16_t _color;
        float _value;
        uint8_t _dataType;

    public:
        TguiElement(){};
        ~TguiElement(){};
        virtual void init(){};
        virtual void update(uint16_t value){};
        void drawBorder();
        Sensor *_sensor;
        Adafruit_GFX *screen;
};

class ProgressBar : public TguiElement
{
    private:
        uint8_t _progress;
        uint16_t _dataScaleRatio;
        Size _block;
        uint16_t _increment;

    public:
        ProgressBar(
            Location loc,
            Size size,
            Size block,
            uint16_t increment,
            uint16_t color,
            Sensor *sensor,
            uint16_t ratio,
            uint8_t dataType);
        void init();
        void update();
        void drawBlocks(uint8_t previousProgress, uint8_t progress);
};

class RunningChart : public TguiElement
{
    private:
        /* data */
    public:
};

class FramedText : public TguiElement
{
    private:
        /* data */
    public:
};

class Label : public TguiElement
{
private:
    const char *_unit;
    uint8_t _textSize;
    uint8_t _unitSize;
    bool _unitLocation;
    uint8_t _nDigitMax;
    bool _onlyInteger;

public:
    Label(
        Location loc,
        uint16_t color,
        Sensor *sensor,
        const char *unit,
        uint8_t textSize,
        uint8_t unitSize,
        bool onlyInteger = ONLY_INTEGER,
        uint8_t nDigitMax = 4,
        bool unitLocation = DRAW_ON_RIGHT,
        uint8_t dataType = 0);
    void init();
    void update();
    void drawUnit();
    void drawDigits(float value);
    void drawDigits(int value);
    void drawPadding(uint8_t nDigits);

    enum
    {
        DRAW_ON_BOTTOM,
        DRAW_ON_RIGHT
    };

    enum
    {
        HAS_DECIMAL = 0,
        ONLY_INTEGER = 1
    };
};