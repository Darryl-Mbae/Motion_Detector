#include "AlarmController.h"
#include "Config.h"

void AlarmController::begin() {

    Serial.println();
    Serial.println("[BOOT] Motion Alarm starting...");

    // Settings
    if (!settings.begin()) {
        Serial.println("[ERROR] Failed to open settings storage.");
    }

    const bool savedIncognito = settings.getIncognito();

    // Hardware
    button.begin(PIN_BTN);
    pir.begin(PIN_PIR);

    leds.begin(
        PIN_LED_R,
        PIN_LED_G,
        PIN_LED_B,
        PIN_LED_YELLOW
    );

    leds.setIncognito(savedIncognito);

    Serial.printf(
        "[BOOT] Incognito: %s\n",
        savedIncognito ? "ON" : "OFF"
    );

    enterState(SystemState::BOOT_SELFTEST);
}

void AlarmController::update() {

    // Always process button.
    button.update();

    handleButton();

    // Software standby
    if (!poweredOn) {
        leds.setPowered(false);
        leds.update();
        return;
    }

    leds.setPowered(true);

    // Read PIR
    pir.update();

    handleAlarmLogic();

    updateOutputs();
}

void AlarmController::handleButton() {

    if (button.wasLongPressed()) {
        resetConnection();
        return;
    }

    if (button.wasShortPressed()) {
        togglePower();
    }
}

void AlarmController::togglePower() {

    poweredOn = !poweredOn;

    if (!poweredOn) {

        Serial.println("[POWER] Alarm OFF");
        Serial.println("[POWER] Entering software standby.");

        enterState(SystemState::POWERED_OFF);

        leds.setPowered(false);

    } else {

        Serial.println("[POWER] Alarm ON");

        // Start fresh boot/self-test sequence.
        enterState(SystemState::BOOT_SELFTEST);

        leds.setPowered(true);
    }
}

void AlarmController::resetConnection() {

    Serial.println("[CONNECTION] 5-second reset requested.");

    /*
      Phase 2 implementation:

      1. Stop current network connection.
      2. Enter BLE provisioning mode.
      3. Allow phone to connect.
      4. Receive new Wi-Fi credentials.
      5. Save credentials.
      6. Connect to Wi-Fi.
      7. Continue to server.

      BLE does not exist yet, so this is intentionally
      only a placeholder.
    */
}

void AlarmController::handleAlarmLogic() {

    const bool pirFault = pir.hasFault();

    switch (currentState) {

        case SystemState::BOOT_SELFTEST:

            if (millis() - stateEnteredAt >= BOOT_SELFTEST_MS) {
                enterState(SystemState::IDLE);
            }

            break;

        case SystemState::IDLE:

            // Check health state before checking for motion.
            // If a fault develops (network down or PIR fault),
            // transition to appropriate fault state immediately.
            updateHealthState(pirFault);

            // Only check for motion if no faults exist.
            if (currentState == SystemState::IDLE && pir.motionDetected()) {

                Serial.println("[EVENT] Motion detected");

                enterState(SystemState::ALERT);
            }

            break;

        case SystemState::ALERT:

            // During an alert, check if a FAULT condition develops.
            // We do NOT call updateHealthState() because that would
            // exit ALERT prematurely if the PIR signal drops (normal behavior).
            // Instead, only transition to a fault state if a real fault is detected.
            if (pirFault) {
                enterState(SystemState::PIR_FAULT);
            }
            else if (
                millis() - stateEnteredAt >= ALERT_DURATION_MS
            ) {
                // Alert timeout: return to health check
                updateHealthState(pirFault);
            }

            break;

        case SystemState::PIR_FAULT:

        case SystemState::NET_DOWN:

        case SystemState::BOTH_FAULT:

            updateHealthState(pirFault);

            break;

        case SystemState::POWERED_OFF:
            break;
    }
}

void AlarmController::updateHealthState(bool pirFault) {

    if (pirFault && !internetConnected) {
        enterState(SystemState::BOTH_FAULT);
    }
    else if (pirFault) {
        enterState(SystemState::PIR_FAULT);
    }
    else if (!internetConnected) {
        enterState(SystemState::NET_DOWN);
    }
    else {
        enterState(SystemState::IDLE);
    }
}

void AlarmController::updateOutputs() {

    switch (currentState) {

        case SystemState::BOOT_SELFTEST:
            leds.setStatus(LEDStatus::BOOT);
            break;

        case SystemState::IDLE:
        case SystemState::ALERT:
            leds.setStatus(LEDStatus::NORMAL);
            break;

        case SystemState::NET_DOWN:
            leds.setStatus(LEDStatus::INTERNET_FAULT);
            break;

        case SystemState::PIR_FAULT:
            leds.setStatus(LEDStatus::PIR_FAULT);
            break;

        case SystemState::BOTH_FAULT:
            leds.setStatus(LEDStatus::BOTH_FAULT);
            break;

        case SystemState::POWERED_OFF:
            leds.setStatus(LEDStatus::OFF);
            break;
    }

    leds.setMotionAlert(
        currentState == SystemState::ALERT
    );

    leds.update();
}

void AlarmController::setIncognito(bool enabled) {

    settings.setIncognito(enabled);

    leds.setIncognito(enabled);

    Serial.printf(
        "[MODE] Incognito %s\n",
        enabled ? "ON" : "OFF"
    );
}

void AlarmController::setInternetConnected(bool connected) {

    wifiInitialized = true;  // WiFi layer has made a determination

    if (internetConnected == connected) {
        return;
    }

    internetConnected = connected;

    Serial.printf(
        "[NETWORK] Internet %s\n",
        connected ? "CONNECTED" : "DOWN"
    );
}

SystemState AlarmController::getState() const {
    return currentState;
}

void AlarmController::enterState(SystemState newState) {

    if (newState == currentState) {
        return;
    }

    currentState = newState;
    stateEnteredAt = millis();

    Serial.printf(
        "[STATE] -> %d at %lu ms\n",
        static_cast<int>(currentState),
        stateEnteredAt
    );
}