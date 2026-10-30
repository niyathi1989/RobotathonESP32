#pragma once
#include <Arduino.h>

// ============================================================
//  APDS-9960 COLOR SENSOR (I2C)
//    VIN -> 3.3V  (NOT 5V!)
//    GND -> GND
//    SDA -> GPIO 21
//    SCL -> GPIO 22
// ============================================================

enum DetectedColor {
    COLOR_NONE,      // too dark / nothing in front of the sensor
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_UNKNOWN    // light detected, but no clear winner (white, gray, yellow...)
};

void colorSensorInit();                 // call once in setup()
void colorSensorUpdate();               // call every loop(); never blocks
DetectedColor colorSensorGetColor();    // latest classified color
const char* colorName(DetectedColor c);