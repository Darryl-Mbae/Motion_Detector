#pragma once

#include <Arduino.h>

class Button {
public:
    void begin(uint8_t pin);

    void update();

    bool wasShortPressed();
    bool wasLongPressed();

private:
    uint8_t pin = 0;

    bool rawLast = HIGH;
    bool stableState = HIGH;

    unsigned long lastRawChange = 0;
    unsigned long pressStartedAt = 0;

    bool longPressFired = false;

    bool shortPressEvent = false;
    bool longPressEvent = false;
};