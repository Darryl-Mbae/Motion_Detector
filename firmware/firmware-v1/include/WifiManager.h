#pragma once

#include <Arduino.h>

/*
    ============================================================
    WifiManager.h
    ============================================================

    Responsible for:

        - Starting Wi-Fi
        - Monitoring the Wi-Fi connection
        - Detecting disconnects
        - Automatically reconnecting
        - Reporting connection status
        - Providing the current connection state

    IMPORTANT:

    This class does NOT:
        - Send data to the server
        - Handle HTTP/HTTPS
        - Handle MQTT
        - Handle BLE
        - Handle notifications

    Those will be separate modules.

    Architecture:

        AlarmController
              │
              ▼
        WifiManager
              │
              ▼
            Wi-Fi
              │
              ▼
          Internet
    ============================================================
*/

class WifiManager
{
public:

    /*
        Start the Wi-Fi manager.

        This starts the initial connection attempt.

        IMPORTANT:
        begin() does NOT wait until Wi-Fi connects.
        The connection happens asynchronously while the
        rest of the firmware continues running.
    */
    void begin();


    /*
        Must be called repeatedly from the main loop.

        This checks the current Wi-Fi state and handles
        reconnect attempts when necessary.
    */
    void update();


    /*
        Returns true when the ESP32 is currently connected
        to a Wi-Fi access point.
    */
    bool isConnected() const;


    /*
        Returns the current Wi-Fi signal strength.

        RSSI is measured in dBm.

        Example:

            -45  = strong
            -70  = usable
            -85  = weak
    */
    int getRSSI() const;


    /*
        Returns the local IP address assigned to the ESP32.

        Example:

            192.168.1.42
    */
    String getIPAddress() const;


    /*
        Returns how many connection attempts have been made
        since the manager started.
    */
    unsigned long getReconnectAttempts() const;

private:

    /*
        Internal Wi-Fi state.

        We keep this private because other modules don't need
        to manipulate Wi-Fi state directly.
    */
    bool connected = false;

    /*
        Time when the last connection attempt started.
    */
    unsigned long connectionStartedAt = 0;

    /*
        Time when the last reconnect attempt was made.
    */
    unsigned long lastReconnectAttempt = 0;

    /*
        Number of reconnect attempts.

        Useful later for diagnostics/telemetry.
    */
    unsigned long reconnectAttempts = 0;


    /*
        Prevents us from continuously calling WiFi.begin()
        every single loop iteration.
    */
    bool connectionInProgress = false;


    /*
        Start a Wi-Fi connection attempt.
    */
    void startConnection();


    /*
        Handle a newly established connection.
    */
    void handleConnected();


    /*
        Handle a lost connection.
    */
    void handleDisconnected();
};