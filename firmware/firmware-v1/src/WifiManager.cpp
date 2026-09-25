#include "WifiManager.h"

#include <WiFi.h>

/*
    ============================================================
    Wi-Fi Configuration
    ============================================================

    TEMPORARY DEVELOPMENT CONFIGURATION

    We are intentionally hardcoding these credentials for now.

    Later:

        Hardcoded credentials
                ↓
        BLE provisioning
                ↓
        Credentials stored in NVS

    DO NOT commit real Wi-Fi credentials to a public GitHub
    repository.
    ============================================================
*/

namespace
{

    constexpr const char* WIFI_SSID = "Wokwi-GUEST";
    constexpr const char* WIFI_PASSWORD = "";



    /*
        How long we allow a connection attempt to remain
        in progress before trying again.

        15 seconds is long enough for most normal connections
        without making the device appear frozen.
    */
    constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;


    /*
        Time between reconnect attempts.

        We deliberately don't hammer the access point with
        continuous connection attempts.
    */
    constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 5000;
}


/*
    ============================================================
    begin()
    ============================================================

    Starts the Wi-Fi manager.

    We configure the ESP32 as a Wi-Fi station:

        ESP32
          ↓
        Wi-Fi router

    NOT:

        ESP32 → Access Point

    The ESP32 will connect to your existing router.
    ============================================================
*/

void WifiManager::begin()
{
    Serial.println();
    Serial.println("[WIFI] Starting Wi-Fi manager...");

    /*
        Make sure our internal state starts as disconnected.
    */
    connected = false;
    connectionInProgress = false;
    reconnectAttempts = 0;

    /*
        ESP32 will operate as a Wi-Fi station.

        STA = connect to an existing Wi-Fi network.
    */
    WiFi.mode(WIFI_STA);

    /*
        Start the first connection attempt.
    */
    startConnection();
}


/*
    ============================================================
    update()
    ============================================================

    This function is called repeatedly from main.cpp.

    The important design principle here is:

        NEVER do this:

            while (WiFi.status() != WL_CONNECTED)
            {
                // wait...
            }

    That would block the alarm system.

    Instead:

        Check
        ↓
        Do a little work
        ↓
        Return
        ↓
        Main loop continues
    ============================================================
*/

void WifiManager::update()
{
    const unsigned long now = millis();

    const wl_status_t wifiStatus = WiFi.status();


    /*
        --------------------------------------------------------
        CASE 1
        Wi-Fi is connected
        --------------------------------------------------------
    */

    if (wifiStatus == WL_CONNECTED)
    {
        /*
            If we previously thought we weren't connected,
            this is a NEW connection.
        */
        if (!connected)
        {
            handleConnected();
        }

        return;
    }


    /*
        --------------------------------------------------------
        CASE 2
        Wi-Fi is not connected
        --------------------------------------------------------
    */

    /*
        If we previously thought we were connected but Wi-Fi
        has now disappeared, report the disconnect.
    */
    if (connected)
    {
        handleDisconnected();
    }


    /*
        --------------------------------------------------------
        CASE 3
        Connection attempt timed out
        --------------------------------------------------------
    */

    if (connectionInProgress)
    {
        if ((now - connectionStartedAt) >= WIFI_CONNECT_TIMEOUT_MS)
        {
            Serial.println("[WIFI] Connection attempt timed out.");

            /*
                Stop the current attempt.
            */
            WiFi.disconnect();

            connectionInProgress = false;
        }

        return;
    }


    /*
        --------------------------------------------------------
        CASE 4
        Start another connection attempt
        --------------------------------------------------------
    */

    if ((now - lastReconnectAttempt) >= WIFI_RECONNECT_INTERVAL_MS)
    {
        startConnection();
    }
}


/*
    ============================================================
    startConnection()
    ============================================================

    Starts a new connection attempt.

    Notice that we don't wait here.

    The ESP32 Wi-Fi stack continues trying in the background.
    ============================================================
*/

void WifiManager::startConnection()
{
    const unsigned long now = millis();

    /*
        Prevent duplicate connection attempts.
    */
    if (connectionInProgress)
    {
        return;
    }


    Serial.println("[WIFI] Connecting...");
    Serial.printf("[WIFI] SSID: %s\n", WIFI_SSID);


    /*
        Start Wi-Fi connection.

        This is intentionally non-blocking.
    */
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


    connectionStartedAt = now;
    lastReconnectAttempt = now;

    connectionInProgress = true;

    reconnectAttempts++;


    Serial.printf(
        "[WIFI] Connection attempt #%lu\n",
        reconnectAttempts
    );
}


/*
    ============================================================
    handleConnected()
    ============================================================

    Called once when Wi-Fi becomes connected.
    ============================================================
*/

void WifiManager::handleConnected()
{
    connected = true;
    connectionInProgress = false;


    Serial.println();
    Serial.println("[WIFI] ================================");
    Serial.println("[WIFI] Connected!");
    Serial.printf("[WIFI] SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("[WIFI] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WIFI] RSSI: %d dBm\n", WiFi.RSSI());
    Serial.println("[WIFI] ================================");
    Serial.println();
}


/*
    ============================================================
    handleDisconnected()
    ============================================================

    Called when an existing Wi-Fi connection disappears.
    ============================================================
*/

void WifiManager::handleDisconnected()
{
    connected = false;

    Serial.println("[WIFI] Connection lost.");
}


/*
    ============================================================
    isConnected()
    ============================================================

    Public method used by other modules.

    Example:

        if (wifi.isConnected())
        {
            // Send data to server
        }
    ============================================================
*/

bool WifiManager::isConnected() const
{
    return connected;
}


/*
    ============================================================
    getRSSI()
    ============================================================

    Returns Wi-Fi signal strength.

    Only meaningful when connected.
    ============================================================
*/

int WifiManager::getRSSI() const
{
    if (!connected)
    {
        return 0;
    }

    return WiFi.RSSI();
}


/*
    ============================================================
    getIPAddress()
    ============================================================

    Returns the ESP32's local IP address.
    ============================================================
*/

String WifiManager::getIPAddress() const
{
    if (!connected)
    {
        return "";
    }

    return WiFi.localIP().toString();
}


/*
    ============================================================
    getReconnectAttempts()
    ============================================================

    Useful later for diagnostics.

    Example:

        Device connected  → 1 attempt
        Router disappears → reconnect
        Router returns    → 2 attempts

    Later this could be included in device telemetry.
    ============================================================
*/

unsigned long WifiManager::getReconnectAttempts() const
{
    return reconnectAttempts;
}