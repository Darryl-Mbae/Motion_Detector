#pragma once

#include <Arduino.h>

class PirSensor {
public:
    void begin(uint8_t pin);

    void update();

    bool motionDetected();
    bool hasFault();

private:
    uint8_t pin = 0;

    bool lastState = LOW;

    bool highTiming = false;
    unsigned long highSince = 0;

    bool motionEvent = false;
};