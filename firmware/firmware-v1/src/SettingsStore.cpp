#include "SettingsStore.h"

/*
    SettingsStore.cpp

    This file contains the actual implementation of the
    SettingsStore class declared in SettingsStore.h.

    SettingsStore is responsible for persistent settings.

    Persistent means:
        - The ESP32 is powered off
        - The ESP32 boots again
        - The setting is still there

    We currently only store:
        - incognito mode

    ESP32 Preferences uses NVS (Non-Volatile Storage).
    NVS is flash storage built into the ESP32.
*/

namespace
{
    // NVS namespace for our application.
    // Think of this like a small folder/database section.
    constexpr const char* NAMESPACE = "motionalarm";

    // Name of the value we store.
    constexpr const char* KEY_INCOGNITO = "incognito";
}


/*
    Open our NVS storage namespace.

    false = read/write mode.

    Returns:
        true  -> storage opened successfully
        false -> something went wrong
*/
bool SettingsStore::begin()
{
    return preferences.begin(NAMESPACE, false);
}


/*
    Read the saved incognito setting.

    If no value has ever been saved, return false.

    That means:

        First boot:
            incognito = OFF

        Later boots:
            restore whatever the user previously selected
*/
bool SettingsStore::getIncognito()
{
    return preferences.getBool(KEY_INCOGNITO, false);
}


/*
    Save the incognito setting.

    Example:

        setIncognito(true);

    stores:

        incognito = ON

    Even after rebooting the ESP32, getIncognito()
    will return true.
*/
void SettingsStore::setIncognito(bool enabled)
{
    preferences.putBool(KEY_INCOGNITO, enabled);
}