/*!
 * @file tgui-common.h
 *
 * Written by Wyng AB Sweden, visit us http://www.nordicalliance.eu
 *
 * Apache license.
 *
 */
#pragma once

#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define DEBUG
#ifdef DEBUG
#define Sprintln(a) Serial.println(a)
#define Sprint(a) Serial.print(a)
#else
#define Sprintln(a)
#define Sprint(a)
#endif

/* COMMANDS */


/* Parameters */


/* REGISTERS */


typedef struct Size
{
    uint16_t width;
    uint16_t height;
} Size;

typedef struct Range
{
    uint16_t low;
    uint16_t high;
} Range;

typedef struct Location
{
    uint16_t x;
    uint16_t y;
} Location;

class Sensor
{
protected:
    uint8_t _filterSize;
    virtual void addDataPoint(uint8_t channel, float data){};

public:
    Sensor(){};
    ~Sensor(){};
    virtual void init(){};
    uint16_t _reportInterval;
    virtual float readDataPoint(uint8_t channel = 0, bool getRawData = false) { return 0; };
    virtual uint16_t getParameters(uint16_t input) { return input; };
};