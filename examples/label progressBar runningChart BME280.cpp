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

#define USE_BME280  1
#define USE_BATTERY 1

const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;

#ifdef USE_BME280
SensorBME280 bme = SensorBME280(0x76, 250);
void bmeGetData()
{
    bme.updateHumidity();
    bme.updateTemperature();
    bme.updatePressure();
    bme.updateAltitude();
}
Ticker bmeEvent(bmeGetData, bme._reportInterval, 0);

Label humidityLable = Label(
    {20, 50},
    foregroundColor,
    &bme,
    "%",
    5,
    2,
    Label::HAS_DECIMAL,
    4,
    Label::DRAW_ON_BOTTOM,
    BME280_HUMIDITY);
Label temperatureLable = Label(
    {20, 120},
    foregroundColor,
    &bme,
    "c",
    5,
    2,
    Label::HAS_DECIMAL,
    4,
    Label::DRAW_ON_BOTTOM,
    BME280_TEMPERATURE);
Label pressureLable = Label(
    {160, 50},
    foregroundColor,
    &bme,
    "hPa",
    5,
    2,
    Label::HAS_DECIMAL,
    5,
    Label::DRAW_ON_BOTTOM,
    BME280_PRESSURE);
Label altitudeLable = Label(
    {160, 120},
    foregroundColor,
    &bme,
    "m",
    5,
    2,
    Label::HAS_DECIMAL,
    5,
    Label::DRAW_ON_BOTTOM,
    BME280_ALTITUDE);

uint32_t bmeEventPreviousCounter = 0;
RunningChart humidityChart = RunningChart(
    {10, 190},
    {300, 45},
    3,
    foregroundColor,
    &bme,
    BME280_HUMIDITY,
    100,
    0);
#endif

#ifdef USE_BATTERY
SensorBattery battery = SensorBattery(1000);
void batteryGetData()
{
    battery.updateLevel();
    battery.updateVoltage();
}
Ticker batteryEvent(batteryGetData, battery._reportInterval, 0);

ProgressBar batteryPbar = ProgressBar(
    {278, 10},
    {32, 10},
    {6, 10},
    8,
    foregroundColor,
    &battery,
    1,
    BATTERY_LEVEL);
Label batteryVoltageLable = Label(
    {200, 10},
    foregroundColor,
    &battery,
    "mv",
    2,
    1,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    BATTERY_VOLTAGE);
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

#ifdef USE_BATTERY
    battery.init();
    batteryPbar.init();
    batteryVoltageLable.init();
    batteryEvent.start();
#endif

#ifdef USE_BME280
    bme.init();
    humidityLable.init();
    temperatureLable.init();
    pressureLable.init();
    altitudeLable.init();
    humidityChart.init();
    bmeEvent.start();
#endif
}

void loop(void)
{
#ifdef USE_BATTERY
    batteryEvent.update();
    batteryPbar.update();
    batteryVoltageLable.update();
#endif

#ifdef USE_BME280
    bmeEvent.update();
    humidityLable.update();
    temperatureLable.update();
    pressureLable.update();
    altitudeLable.update();
    if(bmeEvent.counter() != bmeEventPreviousCounter)
    {
        humidityChart.update();
        bmeEventPreviousCounter = bmeEvent.counter();
    }
#endif
}
