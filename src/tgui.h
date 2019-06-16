/*!
 * @file tgui.h
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

#include <tgui-common.h>

#include <Adafruit_GFX.h>


/* REGISTERS */

/* Parameters */
#define foregroundColor 0xFFE0 //ILI9340_YELLOW
#define backgroundColor 0x0016 //0x001F ILI9340_BLUE

void InitializeScreen();

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