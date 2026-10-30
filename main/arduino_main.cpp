// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 Ricardo Quesada
// http://retro.moe/unijoysticle2

#include "sdkconfig.h"
#include <Arduino.h>
#include <Bluepad32.h>
#include <uni.h>
#include "controller_callbacks.h"
#include "motors.h"
#include "color_sensor.h"

// ============================================================
//  SETTINGS
// ============================================================

const int STICK_DEADZONE = 40;    // ignore tiny stick drift (stick range is about -511..512)
const int STICK_MAX      = 512;

const unsigned long PRINT_INTERVAL_MS = 200;   // how often stick/motor values print to the terminal

// Press D-pad RIGHT to switch between the two drive modes.
enum DriveMode {
    DRIVE_TANK,     // left stick = left motor, right stick = right motor
    DRIVE_ARCADE    // left stick only: up/down = forward/back, left/right = turn
};
static DriveMode driveMode = DRIVE_TANK;

// BP32 library allows up to 4 controllers, we only need 1
extern ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ============================================================
//  HELPERS
// ============================================================

// Turns a stick's up/down position into a motor speed (-255..255).
// Stick up is NEGATIVE in Bluepad32, so we flip it: up = forward.
static int stickToSpeed(int axis) {
    if (abs(axis) < STICK_DEADZONE) return 0;

    int magnitude = abs(axis) - STICK_DEADZONE;
    int speed = map(magnitude, 0, STICK_MAX - STICK_DEADZONE, 0, 255);
    speed = constrain(speed, 0, 255);

    return (axis < 0) ? speed : -speed;
}

// Toggles the drive mode once per D-pad RIGHT press (not continuously while held).
static void checkModeToggle(ControllerPtr ctl) {
    static bool wasPressed = false;
    bool pressed = (ctl->dpad() & DPAD_RIGHT) != 0;

    if (pressed && !wasPressed) {
        driveMode = (driveMode == DRIVE_TANK) ? DRIVE_ARCADE : DRIVE_TANK;
        Console.printf("*** DRIVE MODE: %s ***\n", driveMode == DRIVE_TANK ? "TANK (two sticks)"
                                                                          : "ARCADE (left stick only)");
    }
    wasPressed = pressed;
}

// ============================================================
//  DRIVING
// ============================================================

static void updateMotorsFromController() {
    static unsigned long lastPrint = 0;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr ctl = myControllers[i];
        if (ctl && ctl->isConnected()) {
            checkModeToggle(ctl);

            int leftSpeed, rightSpeed;
            int axisA, axisB;   // only for printing

            if (driveMode == DRIVE_TANK) {
                // Left stick Y -> left motor, right stick Y -> right motor
                axisA = ctl->axisY();
                axisB = ctl->axisRY();
                leftSpeed  = stickToSpeed(axisA);
                rightSpeed = stickToSpeed(axisB);
            } else {
                // Left stick only: Y = forward/back, X = turn (right is positive)
                axisA = ctl->axisY();
                axisB = ctl->axisX();
                int forward = stickToSpeed(axisA);
                int turn    = -stickToSpeed(axisB);   // flip so stick right = positive turn
                leftSpeed   = constrain(forward + turn, -255, 255);
                rightSpeed  = constrain(forward - turn, -255, 255);
                Console.printf("Forward: %d Turn: %d\nLeft Speed: %d, Right Speed: %d", forward, turn, leftSpeed, rightSpeed);
                
            }

            motorLeftDrive(leftSpeed);
            motorRightDrive(rightSpeed);

            if (millis() - lastPrint >= PRINT_INTERVAL_MS) {
                lastPrint = millis();
                if (driveMode == DRIVE_TANK) {
                    Console.printf("[TANK]   LEFT stick: %4d -> motor %4d  |  RIGHT stick: %4d -> motor %4d\n",
                                   axisA, leftSpeed, axisB, rightSpeed);
                } else {
                    Console.printf("[ARCADE] stick Y: %4d  X: %4d  ->  LEFT motor %4d  |  RIGHT motor %4d\n",
                                   axisA, axisB, leftSpeed, rightSpeed);
                }
            }
            return;
        }
    }
}

// ============================================================
//  SETUP
// ============================================================

void setup() {
    Serial.begin(115200);

    // Bluepad32 / controller
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    esp_log_level_set("gpio", ESP_LOG_ERROR);   // suppress gpio_isr_service log spam
    uni_bt_allowlist_set_enabled(true);

    motorsInit();
    colorSensorInit();
}

// ============================================================
//  LOOP
// ============================================================

void loop() {
    BP32.update();                  // read the newest controller data
    updateMotorsFromController();   // apply it to the motors immediately
    colorSensorUpdate();            // read the color sensor (non-blocking)

    vTaskDelay(1);                  // one RTOS tick: only here so the watchdog doesn't reset the ESP32
}