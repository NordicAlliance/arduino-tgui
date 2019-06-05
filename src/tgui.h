/*!
 * @file tgui.h
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.com
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
#include <Zforce.h>

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

/* Sensor specific parameters */
#define SEALEVELPRESSURE_HPA (1013.25)
#define DATA_READY 17   //PD2(INT0) on Odroid

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

enum
{
    ZFORCE_TOUCH,
    ZFORCE_X,
    ZFORCE_Y,
    ZFORCE_STATUS,
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

typedef struct Range
{
    uint16_t low;
    uint16_t high;
} Range;

void InitializeScreen();

class Sensor
{
    protected:
        uint8_t _filterSize;
        virtual void addDataPoint(uint8_t channel, float data){};

    public:
        Sensor(){};
        ~Sensor(){};
        virtual void init(){};
        uint16_t _reportInterval;
        virtual float readDataPoint(uint8_t channel, bool getRawData) { return 0; };
        virtual uint16_t getParameters(uint16_t input) { return input; };
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
    void updateLevel();
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

class TguiElement
{
    protected:
        /* data */
        Location _loc;
        Size _size;
        uint16_t _color;
        float _value;
        uint8_t _dataType;
        bool _showBorder;

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
        uint16_t _resolution;

    public:
        ProgressBar(
            Location loc,
            Size size,
            Size block,
            uint16_t resolution,
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
        uint16_t _timepoint;
        uint16_t _resolution;
        uint16_t _dynamicRangeHigh;
        uint16_t _dynamicRangeLow;
        uint16_t scaleValue(float value);
        void drawIndicator();

    public:
        RunningChart(
            Location loc,
            Size size,
            uint16_t resolution,
            uint16_t color,
            Sensor *sensor,
            uint8_t dataType,
            uint16_t dynamicRangeHigh,
            uint16_t dynamicRangeLow);
        void init();
        void update();
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

class XyPlot : public TguiElement
{
    private:
        uint16_t _resolution;
        Range _rangeX;
        Range _rangeY;
        uint8_t _dataTypeX;
        uint8_t _dataTypeY;
        Location _previousLoc;
        bool _redraw;
        bool _keepTrail;
        uint16_t scaleValue(float value, bool axis);
        void drawIndicator(Location* now, Location* before, bool drawNow, bool keepTrail = false);
        bool matchLocation(Location *a, Location *b);

    public:
        XyPlot(
            Location loc,
            Size size,
            uint16_t resolution,
            uint16_t color,
            Sensor *sensor,
            uint8_t dataTypeX,
            Range rangeX,
            uint8_t dataTypeY,
            Range rangeY,
            bool keepTrail = false);
        void init();
        void update();

    enum
    {
        AXIS_X = 0,
        AXIS_Y = 1
    };

    enum
    {
        NO_LOCATION = 0,
        NEW_LOCATION = 1,
        SAME_LOCATION = 2
    };
};