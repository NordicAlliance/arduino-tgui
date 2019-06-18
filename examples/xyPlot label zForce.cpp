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

#define USE_ZFORCE 1

const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;

#ifdef USE_ZFORCE
Touch air = Touch(15);
void airGetData()
{
    air.updateTouch();
}
Ticker airEvent(airGetData, air._reportInterval, 0);

Label airX = Label(
    {10, 10},
    foregroundColor,
    &air,
    "mm",
    2,
    1,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    ZFORCE_X);
Label airY = Label(
    {100, 10},
    foregroundColor,
    &air,
    "mm",
    2,
    1,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    ZFORCE_Y);
void airLabelUpdate()
{
    airX.update();
    airY.update();
}
Ticker airLabelEvent(airLabelUpdate, 500, 0);

uint32_t airEventPreviousCounter = 0;
XyPlot airPlot = XyPlot(
    {10, 40},
    {300, 190},
    1,
    foregroundColor,
    &air,
    ZFORCE_X,
    {0, 1200},
    ZFORCE_Y,
    {0, 1200},
    true);
#endif

void initPins()
{
    pinMode(backlightPin, OUTPUT);
    analogWrite(backlightPin, backlightPwm);

    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, INPUT);
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
}

void setup()
{
    initPins();
    analogReference(INTERNAL);
    Serial.begin(115200);
    Sprintln(F("Tgui showcase"));

    Wire.begin();    // Zforce lib uses a different I2C lib
    InitializeScreen();

#ifdef USE_ZFORCE
    airX.init();
    airY.init();
    airPlot.init();
    air.init();
    airEvent.start();
    airLabelEvent.start();
#endif
}

void loop(void)
{
#ifdef USE_ZFORCE
    airEvent.update();
    airLabelEvent.update();
    if(airEvent.counter() != airEventPreviousCounter)
    {
        airPlot.update();
        airEventPreviousCounter = airEvent.counter();
    }
#endif
}
