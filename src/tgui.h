/*!
 * @file tgui.h
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <tgui-common.h>

/* REGISTERS */

/* Parameters */
#define foregroundColor 0xFFE0 //ILI9340_YELLOW
#define backgroundColor 0x0016 //0x001F ILI9340_BLUE

void InitializeScreen();

//------------------------ Base class ---------------------------------------/
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

//------------------------ Progress Bar ---------------------------------------/
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

//------------------------ Running Chart ---------------------------------------/
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

//------------------------ Label ---------------------------------------/
class Label : public TguiElement
{
private:
    const char *_unit;
    uint8_t _textSize;
    uint8_t _unitSize;
    bool _unitLocation;
    uint8_t _nDigitMax;
    uint8_t _numberType;

public:
    Label(
        Location loc,
        uint16_t color,
        Sensor *sensor,
        const char *unit,
        uint8_t textSize,
        uint8_t unitSize,
        uint8_t numberType = INTEGER_SIGNED,
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
        FLOAT_SIGNED = 0,
        FLOAT_UNSIGNED = 1,
        INTEGER_SIGNED = 2,
        INTEGER_UNSIGNED = 3
    };
};

//------------------------ Round Indicator ------------------------------/
class Indicator : public TguiElement
{
private:
    uint8_t _dataType2;
    float _value2;
    void _drawBorder();

public:
    Indicator(
        Location loc,
        Sensor *sensor,
        uint8_t dataTypeUpper,
        uint8_t dataTypeLower);

    void init();

    void update();
};

//------------------------ XyPlot ---------------------------------------/
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

//------------------------ Textbox ---------------------------------------/
class Textbox : public TguiElement
{
private:
    Location _lastCursor;
    uint8_t _textSize;
    uint8_t padding;
    uint8_t charW;
    uint8_t charH;
    Size canvas;
    uint8_t maxCol;
    uint8_t maxRow;

public:
    Textbox(
        Location loc,
        Size size,
        uint8_t textSize,
        Sensor *sensor);
    void init();
    void nextCursor(uint16_t textLength);
    void setCursor();
    void update(char *buf, uint16_t length);
};

extern Adafruit_ILI9341 tft;
namespace TGUI
{
void InitializeScreen();

uint8_t countDigits(int num);

uint8_t countFloatDigits(float num);

} // namespace TGUI