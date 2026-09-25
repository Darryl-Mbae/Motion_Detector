#include "Button.h"
#include "Config.h"

void Button::begin(uint8_t buttonPin) {
    pin = buttonPin;

    pinMode(pin, INPUT_PULLUP);

    rawLast = digitalRead(pin);
    stableState = rawLast;
    lastRawChange = millis();
}

void Button::update() {
    shortPressEvent = false;
    longPressEvent = false;

    const bool raw = digitalRead(pin);
    const unsigned long now = millis();

    // Detect raw state change
    if (raw != rawLast) {
        rawLast = raw;
        lastRawChange = now;
    }

    // Debounce
    if ((now - lastRawChange) >= DEBOUNCE_MS &&
        raw != stableState) {

        stableState = raw;

        if (stableState == LOW) {
            // Button pressed
            pressStartedAt = now;
            longPressFired = false;
        } else {
            // Button released
            if (!longPressFired) {
                shortPressEvent = true;
            }
        }
    }

    // Five-second hold
    if (stableState == LOW &&
        !longPressFired &&
        (now - pressStartedAt) >= BTN_LONGPRESS_MS) {

        longPressFired = true;
        longPressEvent = true;
    }
}

bool Button::wasShortPressed() {
    return shortPressEvent;
}

bool Button::wasLongPressed() {
    return longPressEvent;
}
