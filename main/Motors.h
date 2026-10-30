#pragma once
#include <Arduino.h>

// ============================================================
//  H-BRIDGE WIRING
//    LEFT  motor: GPIO 17 -> Vin1, GPIO 16 -> Vin2
//    RIGHT motor: GPIO 25 -> Vin3, GPIO 26 -> Vin4
// ============================================================

void motorsInit();                  // call once in setup()
void motorLeftDrive(int speed);     // -255 (reverse) .. 0 (stop) .. +255 (forward)
void motorRightDrive(int speed);    // -255 (reverse) .. 0 (stop) .. +255 (forward)