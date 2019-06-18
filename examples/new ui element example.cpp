/*!
 * @file main.cpp
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

#include <Arduino.h>
#include <Ticker.h>
#include <tgui.h>
#include <tgui-sensors.h>

#include <SPI.h>
#include <Adafruit_ILI9340.h>

const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;
Adafruit_ILI9340 tft = Adafruit_ILI9340(10, 9, 8);

class Indicator : public TguiElement
{
private:
    uint8_t _dataType2;
    float _value2;
    void _drawBorder()
    {
        screen->drawCircle(_loc.x + _size.width/2, _loc.y + _size.height/2, _size.width/2, foregroundColor);
    }

public:
    Indicator(
        Location loc,
        Sensor *sensor,
        uint8_t dataTypeUpper,
        uint8_t dataTypeLower)
        {
            _loc = loc;
            _size = {110, 110};
            _color = foregroundColor;
            _sensor = sensor;
            screen = &tft;
            _dataType = dataTypeUpper;
            _dataType2 = dataTypeLower;
            _value = 0;
        }
    void init()
    {
        _drawBorder();
        screen->setTextSize(3);
        screen->setTextColor(_color, backgroundColor);
    }
    void update()
    {
        float value = _sensor->readDataPoint(_dataType, false);

        if (value != _value)
        {
            _value = value;
            screen->setCursor(_loc.x + 20, _loc.y + 30);
            screen->println((int)value);
        }

        float value2 = _sensor->readDataPoint(_dataType2, false);

        if (value2 != _value2)
        {
            _value2 = value2;
            screen->setCursor(_loc.x + 30, _loc.y + 65);
            screen->println((int)value2);
        }
    }
};

SensorBattery battery = SensorBattery(1000);
void batteryGetData()
{
    battery.updateLevel();
    battery.updateVoltage();
}
Ticker batteryEvent(batteryGetData, battery._reportInterval, 0);

Indicator batteryIndicator = Indicator(
    {50, 50},
    &battery,
    BATTERY_VOLTAGE,
    BATTERY_LEVEL);

void setup()
{
    Serial.begin(115200);
    Sprintln(F("Tgui showcase"));

    pinMode(backlightPin, OUTPUT);
    analogWrite(backlightPin, backlightPwm);
    analogReference(INTERNAL);

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(backgroundColor);

    battery.init();
    batteryIndicator.init();
    batteryEvent.start();
}

void loop(void)
{
    batteryEvent.update();
    batteryIndicator.update();
}
