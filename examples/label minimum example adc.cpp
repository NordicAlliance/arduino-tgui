/*!
 * @file main.cpp
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */

/*
 * In this example you can expect a frame label in the middle of the screen
 * The label displays the value read from A2 and processes as battery voltage
 */

#include <Arduino.h>
#include <Ticker.h>
#include <tgui.h>
#include <tgui-sensors.h>

const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;

SensorBattery battery = SensorBattery(1000);
void batteryGetData()
{
    battery.updateVoltage();
    Sprintln(battery.readDataPoint((uint8_t)BATTERY_VOLTAGE, true));
}
Ticker batteryEvent(batteryGetData, battery._reportInterval, 0);

Label batteryVoltageLable = Label(
    {40, 80},
    foregroundColor,
    &battery,
    "mv",
    8,
    3,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    BATTERY_VOLTAGE);

void setup()
{
    Serial.begin(115200);
    Sprintln(F("Tgui showcase"));

    analogReference(INTERNAL);
    pinMode(backlightPin, OUTPUT);
    analogWrite(backlightPin, backlightPwm);
    InitializeScreen();

    battery.init();
    batteryVoltageLable.init();
    batteryEvent.start();
}

void loop(void)
{
    batteryEvent.update();
    batteryVoltageLable.update();
}
