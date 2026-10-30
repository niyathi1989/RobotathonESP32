#include "color_sensor.h"
#include <Bluepad32.h>
#include <Wire.h>
#include <Arduino_APDS9960.h>

// ============================================================
//  SETTINGS  (tune these for your lighting and distance)
// ============================================================

#define APDS9960_INT  2
#define I2C_SDA       21
#define I2C_SCL       22
#define I2C_FREQ      100000

const bool PRINT_COLOR = false;                 // set true to print color readings to the terminal
const unsigned long PRINT_INTERVAL_MS = 200;

// Classification tuning
const int   MIN_CLEAR     = 20;     // ambient/clear reading below this = "nothing there"
const float MIN_DOMINANCE = 0.42f;  // winning channel must be at least this share of R+G+B (0.33 = even)
const float MIN_MARGIN    = 0.08f;  // ...and beat the runner-up by at least this much

// ============================================================
//  GLOBALS
// ============================================================

static TwoWire I2C_0 = TwoWire(0);
static APDS9960 apds = APDS9960(I2C_0, APDS9960_INT);

static int r = 0, g = 0, b = 0, a = 0;          // latest raw readings
static DetectedColor currentColor = COLOR_NONE;

// ============================================================
//  HELPERS
// ============================================================

const char* colorName(DetectedColor c) {
    switch (c) {
        case COLOR_NONE:    return "NONE";
        case COLOR_RED:     return "RED";
        case COLOR_GREEN:   return "GREEN";
        case COLOR_BLUE:    return "BLUE";
        case COLOR_UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN";
}

// Uses each channel's SHARE of the total, so it still works when the
// overall brightness changes with distance or lighting.
static DetectedColor classify(int red, int green, int blue, int clear) {
    if (clear < MIN_CLEAR) return COLOR_NONE;

    float total = (float)(red + green + blue);
    if (total <= 0) return COLOR_NONE;

    float fr = red / total;
    float fg = green / total;
    float fb = blue / total;

    float best, second;
    DetectedColor winner;

    if (fr >= fg && fr >= fb) {
        winner = COLOR_RED;   best = fr; second = max(fg, fb);
    } else if (fg >= fr && fg >= fb) {
        winner = COLOR_GREEN; best = fg; second = max(fr, fb);
    } else {
        winner = COLOR_BLUE;  best = fb; second = max(fr, fg);
    }

    if (best >= MIN_DOMINANCE && (best - second) >= MIN_MARGIN) return winner;
    return COLOR_UNKNOWN;
}

// ============================================================
//  PUBLIC API
// ============================================================

void colorSensorInit() {
    I2C_0.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
    apds.setInterruptPin(APDS9960_INT);
    apds.begin();   // (set PRINT_COLOR = true to see readings if you need to debug wiring)
}

void colorSensorUpdate() {
    // Non-blocking: only read when a new sample is ready
    if (!apds.colorAvailable()) return;

    apds.readColor(r, g, b, a);
    currentColor = classify(r, g, b, a);

    if (PRINT_COLOR) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint >= PRINT_INTERVAL_MS) {
            lastPrint = millis();
            Console.printf("RED: %d GREEN: %d BLUE: %d AMBIENT: %d  ->  %s\n",
                           r, g, b, a, colorName(currentColor));
        }
    }
}

DetectedColor colorSensorGetColor() {
    return currentColor;
}
