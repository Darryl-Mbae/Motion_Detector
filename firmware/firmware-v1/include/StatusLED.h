#pragma once

#include <Arduino.h>


// ============================================================
// LED STATUS
// ============================================================
//
// Represents the overall health/status shown by the RGB LED.
//
// RGB meanings:
//
// BLUE   = Everything is normal
// GREEN  = Internet/network problem
// YELLOW = PIR problem
// RED    = PIR + internet problems
// WHITE  = Boot self-test
// OFF    = Standby / incognito
//
// ============================================================

enum class LEDStatus {

    OFF,

    BOOT,

    NORMAL,

    INTERNET_FAULT,

    PIR_FAULT,

    BOTH_FAULT
};


// ============================================================
// STATUS LED CLASS
// ============================================================
//
// This class controls:
//   - RGB status LED
//   - Separate yellow motion LED
//
// It does NOT decide WHY something is wrong.
//
// AlarmController decides the system state and tells this
// class what should be displayed.
//
// ============================================================

class StatusLED {

public:

    // --------------------------------------------------------
    // Initialize LED GPIO pins
    // --------------------------------------------------------

    void begin(
        uint8_t redPin,
        uint8_t greenPin,
        uint8_t bluePin,
        uint8_t yellowPin
    );


    // --------------------------------------------------------
    // Refresh LED outputs
    // --------------------------------------------------------

    void update();


    // --------------------------------------------------------
    // Tell the LED controller what system status to display
    // --------------------------------------------------------

    void setStatus(LEDStatus status);


    // --------------------------------------------------------
    // Control the separate yellow motion indicator
    // --------------------------------------------------------

    void setMotionAlert(bool active);


    // --------------------------------------------------------
    // Incognito mode
    //
    // When enabled:
    //   LEDs remain OFF
    //   sensing can continue
    //   networking can continue
    //
    // --------------------------------------------------------

    void setIncognito(bool enabled);


    // --------------------------------------------------------
    // Software power state
    //
    // powered = false:
    //   all LEDs OFF
    //
    // --------------------------------------------------------

    void setPowered(bool powered);


private:

    // --------------------------------------------------------
    // GPIO pins
    // --------------------------------------------------------

    uint8_t redPin = 0;
    uint8_t greenPin = 0;
    uint8_t bluePin = 0;
    uint8_t yellowPin = 0;


    // --------------------------------------------------------
    // Current RGB status
    // --------------------------------------------------------

    LEDStatus status = LEDStatus::OFF;


    // --------------------------------------------------------
    // Additional modes
    // --------------------------------------------------------

    bool motionAlert = false;

    bool incognito = false;

    bool powered = true;


    // --------------------------------------------------------
    // Boot flashing
    // --------------------------------------------------------

    bool bootFlashState = false;

    unsigned long lastFlashToggle = 0;


    // --------------------------------------------------------
    // Internal helper functions
    // --------------------------------------------------------

    void setRGB(
        bool red,
        bool green,
        bool blue
    );

    void turnEverythingOff();
};