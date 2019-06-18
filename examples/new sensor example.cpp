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


const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;

//------------------------- New sensor class --------------------------------/
class SensorAdc : public Sensor
{
private:
    uint8_t _pin;
    RunningMedian _filter = RunningMedian(FILTER_SAMPLE_SIZE);
    void addDataPoint(uint8_t channel, float data)
    {
        _filter.add(data);
    }

public:
    SensorAdc(
        uint8_t pin = A0,
        uint16_t reportInterval = 50)
    {
        _pin = pin;
        _reportInterval = reportInterval;
        _filterSize = 4;
    }
    void init()
    {
        pinMode(_pin, INPUT);
    }
    float readDataPoint(uint8_t channel = 0, bool getRawData = false)
    {
        if (getRawData)
        {
            return _filter.getElement(_filter.getSize() - 1);
        }
        else
        {
            return _filter.getAverage(_filterSize);
        }
    }
    void updateValue()
    {
        addDataPoint(0, analogRead(_pin));
    }
};

//------------------------- Sensor and UI instances -------------------------/
SensorAdc adcPin = SensorAdc(A3, 1);
void adcGetData()
{
    adcPin.updateValue();
}
Ticker adcEvent(adcGetData, adcPin._reportInterval, 0);

Label adcLabel = Label(
    {10, 10},
    foregroundColor,
    &adcPin,
    "mv",
    4,
    2,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    0);

void adcLabelUpdate()
{
    adcLabel.update();
}
Ticker adcLabelEvent(adcLabelUpdate, 250, 0);

RunningChart adcChart = RunningChart(
    {10, 60},
    {300, 170},
    3,
    foregroundColor,
    &adcPin,
    0,
    1023,
    0);

void adcChartUpdate()
{
    adcChart.update();
    Sprintln(adcPin.readDataPoint());
}
Ticker adcChartEvent(adcChartUpdate, 50, 0);

//------------------------- Setup and loop ----------------------------------/
void setup()
{
    Serial.begin(115200);
    Sprintln(F("Tgui showcase"));

    pinMode(backlightPin, OUTPUT);
    analogWrite(backlightPin, backlightPwm);
    InitializeScreen();

    adcPin.init();
    adcChart.init();
    adcLabel.init();
    adcEvent.start();
    adcLabelEvent.start();
    adcChartEvent.start();
}

void loop(void)
{
    adcEvent.update();
    adcChartEvent.update();
    adcLabelEvent.update();
}
