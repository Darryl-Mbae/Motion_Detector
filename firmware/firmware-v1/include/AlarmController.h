#pragma once

#include <Arduino.h>

#include "Button.h"
#include "PirSensor.h"
#include "StatusLED.h"
#include "SettingsStore.h"

enum class SystemState {
    BOOT_SELFTEST,
    IDLE,
    ALERT,
    PIR_FAULT,
    NET_DOWN,
    BOTH_FAULT,
    POWERED_OFF
};

class AlarmController {
public:
    void begin();

    void update();

    // Future BLE/app command
    void setIncognito(bool enabled);

    // Future Wi-Fi module
    void setInternetConnected(bool connected);

    SystemState getState() const;

private:
    Button button;
    PirSensor pir;
    StatusLED leds;
    SettingsStore settings;

    SystemState currentState = SystemState::BOOT_SELFTEST;

    unsigned long stateEnteredAt = 0;

    bool poweredOn = true;
    bool internetConnected = false;  // Start DISCONNECTED, not connected
    bool wifiInitialized = false;     // Track if WiFi has done its first check

    void enterState(SystemState newState);

    void handleButton();
    void handleAlarmLogic();

    void updateHealthState(bool pirFault);
    void updateOutputs();

    void togglePower();

    void resetConnection();
};