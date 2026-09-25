#include "StatusLED.h"
#include "Config.h"


// ============================================================
// BEGIN
// ============================================================

void StatusLED::begin(
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t y
) {

    redPin = r;
    greenPin = g;
    bluePin = b;
    yellowPin = y;


    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(yellowPin, OUTPUT);


    turnEverythingOff();
}


// ============================================================
// SET STATUS
// ============================================================

void StatusLED::setStatus(LEDStatus newStatus) {

    status = newStatus;
}


// ============================================================
// SET MOTION ALERT
// ============================================================

void StatusLED::setMotionAlert(bool active) {

    motionAlert = active;
}


// ============================================================
// SET INCOGNITO
// ============================================================

void StatusLED::setIncognito(bool enabled) {

    incognito = enabled;

    if (incognito) {
        turnEverythingOff();
    }
}


// ============================================================
// SET POWERED
// ============================================================

void StatusLED::setPowered(bool isPowered) {

    powered = isPowered;

    if (!powered) {
        turnEverythingOff();
    }
}


// ============================================================
// TURN EVERYTHING OFF
// ============================================================

void StatusLED::turnEverythingOff() {

    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    digitalWrite(yellowPin, LOW);
}


// ============================================================
// SET RGB
// ============================================================

void StatusLED::setRGB(
    bool red,
    bool green,
    bool blue
) {

    // Software standby or incognito:
    // absolutely no visible LEDs.

    if (!powered || incognito) {

        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);

        return;
    }


    digitalWrite(
        redPin,
        red ? HIGH : LOW
    );

    digitalWrite(
        greenPin,
        green ? HIGH : LOW
    );

    digitalWrite(
        bluePin,
        blue ? HIGH : LOW
    );
}


// ============================================================
// UPDATE
// ============================================================

void StatusLED::update() {

    // --------------------------------------------------------
    // Standby / incognito
    // --------------------------------------------------------

    if (!powered || incognito) {

        turnEverythingOff();

        return;
    }


    // --------------------------------------------------------
    // RGB STATUS
    // --------------------------------------------------------

    switch (status) {

        // ----------------------------------------------------
        // OFF
        // ----------------------------------------------------

        case LEDStatus::OFF:

            setRGB(false, false, false);

            break;


        // ----------------------------------------------------
        // BOOT
        //
        // White flashing self-test
        // ----------------------------------------------------

        case LEDStatus::BOOT: {

            const unsigned long now = millis();


            if (
                now - lastFlashToggle >=
                WHITE_FLASH_MS
            ) {

                bootFlashState = !bootFlashState;

                lastFlashToggle = now;
            }


            setRGB(
                bootFlashState,
                bootFlashState,
                bootFlashState
            );

            break;
        }


        // ----------------------------------------------------
        // NORMAL
        //
        // Blue
        // ----------------------------------------------------

        case LEDStatus::NORMAL:

            setRGB(
                false,
                false,
                true
            );

            break;


        // ----------------------------------------------------
        // INTERNET FAULT
        //
        // Green
        // ----------------------------------------------------

        case LEDStatus::INTERNET_FAULT:

            setRGB(
                false,
                true,
                false
            );

            break;


        // ----------------------------------------------------
        // PIR FAULT
        //
        // Red + Green = Yellow
        // ----------------------------------------------------

        case LEDStatus::PIR_FAULT:

            setRGB(
                true,
                true,
                false
            );

            break;


        // ----------------------------------------------------
        // BOTH FAULTS
        //
        // Red
        // ----------------------------------------------------

        case LEDStatus::BOTH_FAULT:

            setRGB(
                true,
                false,
                false
            );

            break;
    }


    // --------------------------------------------------------
    // SEPARATE YELLOW MOTION LED
    //
    // This is independent from RGB health status.
    // --------------------------------------------------------

    digitalWrite(
        yellowPin,
        motionAlert ? HIGH : LOW
    );
}