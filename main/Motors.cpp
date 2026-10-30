#include "motors.h"
#include "esp_arduino_version.h"

// ============================================================
//  SETTINGS
// ============================================================

// forwardPin is driven for forward, reversePin for reverse.
struct Motor {
    uint8_t forwardPin;
    uint8_t reversePin;
    bool    reversed;     // flip direction without rewiring
    uint8_t pwmChForward; // LEDC channels (only used when USE_PWM = true, core 2.x)
    uint8_t pwmChReverse;
};

static const Motor LEFT_MOTOR  = { 17, 16, false, 0, 1 };   // Vin1 = forward, Vin2 = reverse
static const Motor RIGHT_MOTOR = { 25, 26, false, 2, 3 };   // GPIO25 = Vin3 (forward), GPIO26 = Vin4 (reverse)
// If a wheel spins the wrong way, change its 'false' to 'true'.
// (Motors on opposite sides of a robot are usually mirrored, so one side often needs this.)

// false = plain full ON/OFF (this is what worked on your hardware)
// true  = variable speed with PWM
const bool USE_PWM = false;

const uint32_t PWM_FREQ_HZ = 1000;
const int      MIN_DUTY    = 90;
const uint8_t  PWM_BITS    = 8;

// ============================================================
//  PWM HELPERS (Arduino-ESP32 core 2.x and 3.x)
// ============================================================

static void pwmSetup(uint8_t pin, uint8_t channel) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(pin, PWM_FREQ_HZ, PWM_BITS);
#else
    ledcSetup(channel, PWM_FREQ_HZ, PWM_BITS);
    ledcAttachPin(pin, channel);
#endif
}

static void pwmWrite(uint8_t pin, uint8_t channel, uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(pin, duty);
#else
    ledcWrite(channel, duty);
#endif
}

// ============================================================
//  GENERIC MOTOR DRIVER
// ============================================================

static void motorInit(const Motor& m) {
    if (USE_PWM) {
        pwmSetup(m.forwardPin, m.pwmChForward);
        pwmSetup(m.reversePin, m.pwmChReverse);
    } else {
        pinMode(m.forwardPin, OUTPUT);
        pinMode(m.reversePin, OUTPUT);
        digitalWrite(m.forwardPin, LOW);
        digitalWrite(m.reversePin, LOW);
    }
}

static void motorDrive(const Motor& m, int speed) {
    speed = constrain(speed, -255, 255);
    if (m.reversed) speed = -speed;

    if (USE_PWM) {
        int duty = (speed == 0) ? 0 : MIN_DUTY + (abs(speed) * (255 - MIN_DUTY)) / 255;
        pwmWrite(m.forwardPin, m.pwmChForward, speed > 0 ? duty : 0);
        pwmWrite(m.reversePin, m.pwmChReverse, speed < 0 ? duty : 0);
    } else {
        digitalWrite(m.forwardPin, speed > 0 ? HIGH : LOW);
        digitalWrite(m.reversePin, speed < 0 ? HIGH : LOW);
    }
}

// ============================================================
//  PUBLIC API
// ============================================================

void motorsInit() {
    motorInit(LEFT_MOTOR);
    motorInit(RIGHT_MOTOR);
}

void motorLeftDrive(int speed)  { motorDrive(LEFT_MOTOR, speed); }
void motorRightDrive(int speed) { motorDrive(RIGHT_MOTOR, speed); }