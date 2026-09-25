#include "PirSensor.h"
#include "Config.h"

void PirSensor::begin(uint8_t pirPin) {
    pin = pirPin;

    pinMode(pin, INPUT);

    lastState = digitalRead(pin);
    highTiming = false;
    highSince = 0;
    motionEvent = false;
}

void PirSensor::update() {
    motionEvent = false;

    const bool currentState = digitalRead(pin);
    const unsigned long now = millis();

    // Rising edge = new motion event
    if (currentState == HIGH && lastState == LOW) {
        motionEvent = true;
    }

    // Track continuous HIGH
    if (currentState == HIGH) {
        if (!highTiming) {
            highTiming = true;
            highSince = now;
        }
    } else {
        highTiming = false;
        highSince = 0;
    }

    lastState = currentState;
}

bool PirSensor::motionDetected() {
    return motionEvent;
}

bool PirSensor::hasFault() {
    // Ignore PIR behavior during warm-up.
    if (millis() < PIR_WARMUP_MS) {
        return false;
    }

    if (!highTiming) {
        return false;
    }

    return (millis() - highSince) >= PIR_STUCK_FAULT_MS;
}