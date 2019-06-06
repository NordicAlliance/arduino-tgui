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
#include "tgui.h"


// #define USE_SI1132  1
// #define USE_VL53L0X 1
#define USE_BME280  1
#define USE_BATTERY 1
// #define USE_ZFORCE 1

const uint8_t backlightPin = 5;
uint8_t backlightPwm = 255;

struct sensorList
{
    bool useSi1132 = 0;
    bool useBme280 = 0;
    bool useVl53l0x = 0;
    bool useBattery = 0;
} sensorList;


#ifdef USE_VL53L0X
SensorVL53L0X tof = SensorVL53L0X(0x67, 100);
void tofGetData()
{
    tof.updateData();
}
Ticker tofEvent(tofGetData, tof._reportInterval, 0);

ProgressBar tofPbar = ProgressBar(
    {10, 220},
    {300, 12},
    {4, 12},
    5,
    foregroundColor,
    &tof,
    10,
    VL53L0X_DISTANCE);
Label tofLable = Label(
    {10, 50},
    foregroundColor,
    &tof,
    "mm",
    5,
    2,
    Label::ONLY_INTEGER,
    4,
    Label::DRAW_ON_RIGHT,
    VL53L0X_DISTANCE);
#endif

#ifdef USE_SI1132
SensorSi1132 light = SensorSi1132(0x60, 100);
void ligthGetData()
{
    light.updateIR();
    light.updateUV();
    light.updateVisible();
}
Ticker lightEvent(ligthGetData, light._reportInterval, 0);

ProgressBar lightPbar = ProgressBar(
    {10, 190},
    {300, 8},
    {4, 8},
    5,
    foregroundColor,
    &light,
    20,
    SI1132_VISIBLE);
ProgressBar irPbar = ProgressBar(
    {10, 205},
    {300, 8},
    {4, 8},
    5,
    foregroundColor,
    &light,
    50,
    SI1132_IR);
ProgressBar uvPbar = ProgressBar(
    {10, 220},
    {300, 8},
    {4, 8},
    5,
    foregroundColor,
    &light,
    20,
    SI1132_UV);
Label irLable = Label(
    {10, 50},
    foregroundColor,
    &light,
    "lux",
    5,
    2,
    Label::ONLY_INTEGER,
    6,
    Label::DRAW_ON_BOTTOM,
    SI1132_IR);
#endif

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

// ProgressBar humidityPbar = ProgressBar(
//     {widgetStart, 125},
//     {widgetWidth, 12},
//     {4, 12},
//     5,
//     foregroundColor,
//     &bme,
//     1,
//     BME280_HUMIDITY);
// ProgressBar temperaturePbar = ProgressBar(
//     {widgetStart, 150},
//     {widgetWidth, 12},
//     {4, 12},
//     5,
//     foregroundColor,
//     &bme,
//     1,
//     BME280_TEMPERATURE);
// ProgressBar pressurePbar = ProgressBar(
//     {widgetStart, 175},
//     {widgetWidth, 12},
//     {4, 12},
//     5,
//     foregroundColor,
//     &bme,
//     50,
//     BME280_PRESSURE);
// ProgressBar altitudePbar = ProgressBar(
//     {widgetStart, 200},
//     {widgetWidth, 12},
//     {4, 12},
//     5,
//     foregroundColor,
//     &bme,
//     5,
//     BME280_ALTITUDE);
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

#ifdef USE_BATTERY
    battery.init();
    batteryPbar.init();
    batteryVoltageLable.init();
    batteryEvent.start();
#endif

#ifdef USE_VL53L0X
    tof.init();
    tofPbar.init();
    tofLable.init();
    tofEvent.start();
#endif

#ifdef USE_BME280
    bme.init();
    // temperaturePbar.init();
    // humidityPbar.init();
    // pressurePbar.init();
    // altitudePbar.init();
    humidityLable.init();
    temperatureLable.init();
    pressureLable.init();
    altitudeLable.init();
    humidityChart.init();
    bmeEvent.start();
#endif

#ifdef USE_SI1132
    light.init();
    lightPbar.init();
    irPbar.init();
    uvPbar.init();
    irLable.init();
    lightEvent.start();
#endif

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
#ifdef USE_BATTERY
    batteryEvent.update();
    batteryPbar.update();
    batteryVoltageLable.update();
#endif

#ifdef USE_VL53L0X
    tofEvent.update();
    tofPbar.update();
    tofLable.update();
#endif

#ifdef USE_BME280
    bmeEvent.update();
    // humidityPbar.update();
    // temperaturePbar.update();
    // pressurePbar.update();
    // altitudePbar.update();
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

#ifdef USE_SI1132
    lightEvent.update();
    lightPbar.update();
    irPbar.update();
    uvPbar.update();
    irLable.update();
#endif

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
