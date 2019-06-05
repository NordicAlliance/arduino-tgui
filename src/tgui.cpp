/*!
 * @file tgui.cpp
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.com
 *
 * Apache license.
 *
 */

#include "tgui.h"

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

void InitializeScreen()
{
    tft.begin();
    tft.setRotation(defaultRotation);
    tft.fillScreen(backgroundColor);
}

uint8_t countDigits(int num)
{
    uint8_t count = 0;
    if (num < 0)
    {
        count++;
        num = -num;
    }
    while (num)
    {
        num = num / 10;
        count++;
    }
    return count;
}

uint8_t countFloatDigits(float num)
{
    uint8_t count = 0;
    float threshold = 0.00005;
    while (abs(num - round(num)) > threshold)
    {
        num *= 10.0;
        threshold *= 10.0;
        count++;
    }
    return count;
}

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

//------------------------ Tgui Element ---------------------------------------/
void TguiElement::drawBorder()
{
    screen->drawRoundRect(
        _loc.x - borderPadding * 2,
        _loc.y - borderPadding,
        _size.width + borderPadding * 4,
        _size.height + borderPadding * 2,
        4,
        _color);
}

//------------------------ Progree bar ---------------------------------------/
ProgressBar::ProgressBar(
    Location loc,
    Size size,
    Size block,
    uint16_t resolution,
    uint16_t color,
    Sensor *sensor,
    uint16_t ratio,
    uint8_t dataType = 0)
{
    _loc = loc;
    _size = size;
    _block = block;
    _resolution = resolution;
    _color = color;
    _sensor = sensor;
    _dataType = dataType;
    _dataScaleRatio = ratio;
    _progress = 0;
    screen = &tft,
    _value = 0;
}

void ProgressBar::init()
{
    _progress = 0;
#ifdef pbar_show_border
    drawBorder();
#endif
}

void ProgressBar::drawBlocks(uint8_t previousProgress, uint8_t progress)
{
    const uint8_t totalBlocks = _size.width / _resolution;

    const uint8_t before = totalBlocks * previousProgress / 100;
    const uint8_t after = totalBlocks * progress / 100;
    if(before == after)
        return;

    if(before < after)
    {
        for (uint8_t i = before; i < after; i++)
        {
            screen->fillRect(
                _loc.x + _resolution * i,
                _loc.y,
                _block.width,
                _block.height,
                _color);
        }
    }
    else
    {
        for (uint8_t i = after; i < before; i++)
        {
            screen->fillRect(
                _loc.x + _resolution * i,
                _loc.y,
                _block.width,
                _block.height,
                backgroundColor);
        }
    }
}

void ProgressBar::update()
{
    float value = _sensor->readDataPoint(_dataType, false);

    if (value < 0)  // for now we don't take negtive values
        return;

    if (value == _value)
        return;

    _value = value;

    uint8_t progress = (value / _dataScaleRatio) > 100 ? \
        100 : (value / _dataScaleRatio);
    
    if (progress == _progress)
        return;

    drawBlocks(_progress, progress);
    _progress = progress;
}

//------------------------ Label ---------------------------------------/
Label::Label(
    Location loc,
    uint16_t color,
    Sensor *sensor,
    const char *unit,
    uint8_t textSize,
    uint8_t unitSize,
    bool onlyInteger,
    uint8_t nDigitMax,
    bool unitLocation,
    uint8_t dataType)
{
    _loc = loc;
    _color = color;
    _sensor = sensor;
    _value = 0;
    _dataType = dataType;
    screen = &tft,
    _unit = unit;
    _textSize = textSize;
    _unitSize = unitSize;
    _nDigitMax = nDigitMax;
    _size.height = (unitLocation == DRAW_ON_BOTTOM) ? textPixelH(textSize) + textPixelH(unitSize) : textPixelH(textSize);
    _size.width = (unitLocation == DRAW_ON_BOTTOM) ? textPixelW(textSize) * nDigitMax : textPixelW(textSize) * nDigitMax + textPixelH(unitSize) * strlen(unit);
    _unitLocation = unitLocation;
    _onlyInteger = onlyInteger;
}

void Label::drawDigits(int value)
{
    screen->setTextSize(_textSize);
    screen->setTextColor(_color, backgroundColor);
    screen->setCursor(_loc.x, _loc.y);

    uint8_t nDigits = countDigits(value);

    if(nDigits <= _nDigitMax)
    {
        screen->print(value);
        if(nDigits != _nDigitMax)
        {
            drawPadding(nDigits);
        }
    }
    else
    {
        screen->print("-");
        drawPadding(1);
    }
}

void Label::drawDigits(float value)
{
    screen->setTextSize(_textSize);
    screen->setTextColor(_color, backgroundColor);
    screen->setCursor(_loc.x, _loc.y);

    uint8_t nInteger = countDigits((int)value);

    if((value == (int)value) || (nInteger + 2 > _nDigitMax))
    {
        drawDigits((int)value);
    }
    else
    {
        screen->print(value, _nDigitMax - 1 - nInteger);
    }
}

void Label::drawUnit()
{
    screen->setTextSize(_unitSize);
    screen->setTextColor(_color, backgroundColor);
    if (_unitLocation == DRAW_ON_BOTTOM)
    {
        screen->setCursor(_loc.x, _loc.y + textPixelH(_textSize));
    }
    else
    {
        screen->setCursor(_loc.x + _size.width - strlen(_unit) * textPixelW(_unitSize), _loc.y);
    }
        
    screen->print(_unit);
}

void Label::drawPadding(uint8_t nDigits)
{
    const uint16_t textWidth = textPixelW(_textSize);
    int16_t paddingLength = _size.width - nDigits * textWidth;
    if (_unitLocation == DRAW_ON_RIGHT)
    {
        paddingLength -= strlen(_unit) * textPixelW(_unitSize);
    }

    if (paddingLength >= (int)textWidth)
    {
        screen->setTextSize(_textSize);
        screen->setCursor(_loc.x + nDigits * textWidth, _loc.y);
        for (uint8_t i = 0; i < (paddingLength / textWidth); i++)
        {
            screen->print(" ");
        }
    }
}

void Label::init()
{
    drawBorder();
    drawUnit();
}

void Label::update()
{
    float value = _sensor->readDataPoint(_dataType, false);

    if (value == _value)
        return;

    _value = value;

    if(_onlyInteger == true)
    {
        drawDigits((int)value);
    }
    else
    {
        drawDigits(value);
    }
}

//------------------------ Running Chart ---------------------------------------/
RunningChart::RunningChart(
            Location loc,
            Size size,
            uint16_t resolution,
            uint16_t color,
            Sensor *sensor,
            uint8_t dataType,
            uint16_t dynamicRangeHigh,
            uint16_t dynamicRangeLow)
{
    _loc = loc;
    _size = size;
    _resolution = resolution;
    _color = color;
    _sensor = sensor;
    _dataType = dataType;
    _dynamicRangeHigh = dynamicRangeHigh;
    _dynamicRangeLow = dynamicRangeLow;
    screen = &tft,
    _value = 0;
    _timepoint = 0;
}

void RunningChart::drawIndicator()
{
    uint16_t previousPoint = _timepoint - 1;
    if(_timepoint == 0)
    {
        previousPoint = _size.width / _resolution - 1;
    }

    screen->drawTriangle(
        _loc.x + _resolution * previousPoint - 4,
        _loc.y - 7,
        _loc.x + _resolution * previousPoint,
        _loc.y - 3,
        _loc.x + _resolution * previousPoint + 4,
        _loc.y - 7,
        backgroundColor);

    screen->drawTriangle(
        _loc.x + _resolution * _timepoint - 4,
        _loc.y - 7,
        _loc.x + _resolution * _timepoint,
        _loc.y - 3,
        _loc.x + _resolution * _timepoint + 4,
        _loc.y - 7,
        _color);
}

uint16_t RunningChart::scaleValue(float value)
{
    int pos = ((float)value - _dynamicRangeLow)*_size.height/(_dynamicRangeHigh - _dynamicRangeLow);

    return pos;
}

void RunningChart::init()
{
    drawBorder();
    _timepoint = 0;
}

void RunningChart::update()
{
    _value = _sensor->readDataPoint(_dataType, false);

    if(_timepoint++ == (_size.width / _resolution - 1))
    {
        _timepoint = 0;
    }

    uint16_t value = scaleValue(_value);

    screen->fillRect(
        _loc.x + _resolution * _timepoint,
        _loc.y + _size.height - value,
        _resolution,
        value,
        _color);

    screen->fillRect(
        _loc.x + _resolution * _timepoint,
        _loc.y,
        _resolution,
        _size.height - value,
        backgroundColor);

    drawIndicator();
}

//------------------------ XY Plot ---------------------------------------/
XyPlot::XyPlot(
            Location loc,
            Size size,
            uint16_t resolution,
            uint16_t color,
            Sensor *sensor,
            uint8_t dataTypeX,
            Range rangeX,
            uint8_t dataTypeY,
            Range rangeY,
            bool keepTrail)
{
    _loc = loc;
    _size = size;
    _resolution = resolution;
    _color = color;
    _sensor = sensor;
    _dataTypeX = dataTypeX;
    _dataTypeY = dataTypeY;
    _rangeX = rangeX;
    _rangeY = rangeY;
    screen = &tft,
    _value = 0;
    _previousLoc = {0, 0};
    _redraw = true;
    _keepTrail = keepTrail;
}

void XyPlot::drawIndicator(Location* now, Location* before, bool drawNow, bool keepTrail)
{
    if(!keepTrail)
    {
        screen->fillCircle(
            _loc.x + before->x,
            _loc.y + before->y,
            10,
            backgroundColor);
    }

    if(drawNow == true)
    {
        screen->fillCircle(
            _loc.x + now->x,
            _loc.y + now->y,
            10,
            _color);
    }
}

uint16_t XyPlot::scaleValue(float value, bool axis)
{
    Range *range = (axis == AXIS_X) ? &_rangeX : &_rangeY;
    uint16_t* base = (axis == AXIS_X) ? &_size.width : &_size.height;
    return (((float)value - range->low)*(*base)/(range->high - range->low));
}

void XyPlot::init()
{
    drawBorder();
}

#define diff(x, y) (x > y ? (x - y) : (y - x))

bool XyPlot::matchLocation(Location *a, Location *b)
{
    uint8_t threshold = _keepTrail ? 1 : 3;
    if((diff(a->x,b->x) + diff(a->y, b->y)) < threshold)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void XyPlot::update()
{
    Location nowLoc;
    nowLoc.x = scaleValue(_sensor->readDataPoint(_dataTypeX, false), AXIS_X);
    nowLoc.y = scaleValue(_sensor->readDataPoint(_dataTypeY, false), AXIS_Y);

    uint8_t state = SAME_LOCATION;
    if(_sensor->getParameters(state))
    {
        if(_redraw)
        {
            state = NO_LOCATION;
            _redraw = false;
        }
    }
    else if(!matchLocation(&nowLoc, &_previousLoc))
    {
        state = NEW_LOCATION;
        _redraw = true;
    }

    Sprintln(state);

    switch (state)
    {
    case NEW_LOCATION:
        drawIndicator(&nowLoc, &_previousLoc, true, _keepTrail);
        _previousLoc = nowLoc;
        break;
    case NO_LOCATION:
        if(!_keepTrail)
        {
            drawIndicator(&nowLoc, &_previousLoc, false);
        }
        else
        {
            screen->fillRoundRect(_loc.x-1, _loc.y-1, _size.width+2, _size.height+2,5,backgroundColor);
        }
        
    default:
        break;
    }
}