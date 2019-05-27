/*!
 * @file tgui.cpp
 *
 * Written by Wyng AB Sweden, visit us http://www.wyngstudio.com
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
    addDataPoint(BME280_PRESSURE, _phy.readPressure()-98000);
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
    uint16_t increment,
    uint16_t color,
    Sensor *sensor,
    uint16_t ratio,
    uint8_t dataType = 0)
{
    _loc = loc;
    _size = size;
    _block = block;
    _increment = increment;
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
    const uint8_t totalBlocks = _size.width / _increment;

    const uint8_t before = totalBlocks * previousProgress / 100;
    const uint8_t after = totalBlocks * progress / 100;
    if(before == after)
        return;

    if(before < after)
    {
        for (uint8_t i = before; i < after; i++)
        {
            screen->fillRect(
                _loc.x + _increment * i,
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
                _loc.x + _increment * i,
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
    uint8_t nDigitMax,
    bool unitLocation,
    uint8_t dataType,
    uint8_t nDigitsDisplay)
{
    _loc = loc;
    _color = color;
    _sensor = sensor;
    screen = &tft,
    _unit = unit;
    _value = 0;
    _textSize = textSize;
    _unitSize = unitSize;
    _size.height = (unitLocation == DRAW_ON_BOTTOM) ? textPixelH(textSize) + textPixelH(unitSize) : textPixelH(textSize);
    _size.width = (unitLocation == DRAW_ON_BOTTOM) ? textPixelW(textSize) * nDigitMax : textPixelW(textSize) * nDigitMax + textPixelH(unitSize) * strlen(unit);
    _unitLocation = unitLocation;
    _dataType = dataType;
    _nDigitsDisplay = nDigitsDisplay;
}

void Label::drawDigits(float value)
{
    screen->setTextSize(_textSize);
    screen->setTextColor(_color, backgroundColor);
    screen->setCursor(_loc.x, _loc.y);

    if ((countDigits((int)value) * textPixelW(_textSize)) < _size.width)
    {
        screen->print(value, _nDigitsDisplay);
        drawPadding((int)value);
    }
    else
    {
        screen->print("-");
        drawPadding(9);
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

void Label::drawPadding(int value)
{
    const uint16_t textWidth = textPixelW(_textSize);
    int16_t paddingLength = _size.width - countDigits(value) * textWidth;
    if (_unitLocation == DRAW_ON_RIGHT)
    {
        paddingLength -= strlen(_unit) * textPixelW(_unitSize);
    }

    if (paddingLength >= (int)textWidth)
    {
        screen->setTextSize(_textSize);
        screen->setCursor(_loc.x + countDigits(value) * textWidth, _loc.y);
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

    drawDigits(value);
    if (countDigits((int)value) != countDigits((int)_value))
    {
        drawPadding((int)value);
    }

    _value = value;
}
