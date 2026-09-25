#pragma once

#include <Arduino.h>
#include <Preferences.h>

class SettingsStore {
public:
    bool begin();

    bool getIncognito();
    void setIncognito(bool enabled);

private:
    Preferences preferences;
};