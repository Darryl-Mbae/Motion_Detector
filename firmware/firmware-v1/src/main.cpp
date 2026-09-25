/*
  ============================================================
  MOTION ALARM
  ============================================================

  Main application entry point.

  Board:
    ESP32 DevKit-C

  Phase:
    Phase 2 — Wi-Fi connectivity foundation

  Responsibilities of main.cpp:
    - Start serial communication
    - Create application modules
    - Initialize the alarm controller
    - Initialize Wi-Fi
    - Continuously update Wi-Fi
    - Pass Wi-Fi connection changes to AlarmController
    - Continuously run the alarm controller

  IMPORTANT:

    main.cpp should NOT contain hardware logic.

    Hardware and system logic live inside their own modules.

  Application architecture:

                        main.cpp
                       /        \
                      /          \
                     v            v
             AlarmController   WifiManager
                /   |   \           |
               /    |    \          v
              v     v     v       Wi-Fi
           Button  PIR  StatusLED
                       |
                       v
                  RGB + Yellow

             SettingsStore
                   |
                   v
                  NVS


  Future architecture:

                    main.cpp
                  /    |     \
                 /     |      \
                v      v       v
             Alarm   WiFi    BLE
             Logic    |        |
                      |        |
                      v        v
                   API / Provisioning
                      |
                      v
                    Server
                      |
                      v
                EventReporter
                      |
                      v
                 Notifications

  ============================================================
*/

#include <Arduino.h>

#include "AlarmController.h"
#include "WifiManager.h"


// ============================================================
// GLOBAL APPLICATION OBJECTS
// ============================================================

/*
  The AlarmController owns the main alarm logic.

  It manages:
    - Button
    - PIR
    - LED status
    - Power state
    - Alarm state
    - Health state
    - Settings
*/
AlarmController alarmController;


/*
  WifiManager owns Wi-Fi connectivity.

  It manages:
    - Wi-Fi startup
    - Connection attempts
    - Disconnect detection
    - Reconnection
    - Wi-Fi diagnostics

  It does NOT send data to the server.

  Server communication will be handled by a separate
  ApiClient / EventReporter module later.
*/
WifiManager wifiManager;


/*
  Stores the previous Wi-Fi state.

  Why?

  We don't want to repeatedly tell AlarmController:

      "Wi-Fi connected"

  on every loop iteration.

  Instead, we only notify it when the state actually changes.

  Example:

      DISCONNECTED
           |
           v
      CONNECTED

  This is a real state change.

  But:

      CONNECTED
      CONNECTED
      CONNECTED

  does not require repeated notifications.
*/
bool lastWifiConnected = false;


// ============================================================
// SETUP
// ============================================================

void setup()
{
  /*
    ----------------------------------------------------------
    SERIAL COMMUNICATION
    ----------------------------------------------------------

    115200 baud gives us enough speed for useful debugging
    output without slowing the ESP32 down.

    During development, the Serial Monitor lets us see:

      [BOOT] Motion Alarm starting...
      [WIFI] Connecting...
      [WIFI] Connected!
      [STATE] -> IDLE
      [EVENT] Motion detected
      [POWER] Alarm OFF

    Later, we can introduce a proper logging module.
    ----------------------------------------------------------
  */

  Serial.begin(115200);


  /*
    Give the serial interface a moment to initialize.

    This is not required for the ESP32 itself, but it makes
    early boot messages easier to see during development.
  */

  delay(200);


  /*
    ----------------------------------------------------------
    INITIALIZE THE ALARM CONTROLLER
    ----------------------------------------------------------

    AlarmController::begin() is responsible for initializing
    the local alarm system:

      - Settings / NVS
      - Button
      - PIR
      - RGB LED
      - Yellow LED
      - Saved configuration
      - Initial state

    main.cpp deliberately does not know those implementation
    details.

    That's separation of responsibility.
    ----------------------------------------------------------
  */

  alarmController.begin();


  /*
    ----------------------------------------------------------
    INITIALIZE WIFI
    ----------------------------------------------------------

    WifiManager::begin() starts the Wi-Fi connection process.

    IMPORTANT:

    This does NOT wait for Wi-Fi to connect.

    The connection happens asynchronously while the rest of
    the alarm system continues operating.

    This means:

      PIR can still detect motion
      Button can still work
      LEDs can still update

    while Wi-Fi is connecting.
    ----------------------------------------------------------
  */

  wifiManager.begin();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  /*
    ----------------------------------------------------------
    1. UPDATE WIFI
    ----------------------------------------------------------

    WifiManager performs one non-blocking iteration.

    It checks:

      - Is Wi-Fi connected?
      - Did the connection disappear?
      - Has a connection attempt timed out?
      - Is it time to retry?

    It does NOT block the alarm system.
    ----------------------------------------------------------
  */

  wifiManager.update();


  /*
    ----------------------------------------------------------
    2. CHECK WIFI STATE
    ----------------------------------------------------------

    Ask WifiManager for the current connection state.

    Example:

      true  = Wi-Fi connected
      false = Wi-Fi disconnected
  */

  const bool wifiConnected = wifiManager.isConnected();


  /*
    ----------------------------------------------------------
    3. REPORT WIFI STATE CHANGES
    ----------------------------------------------------------

    We only notify AlarmController when Wi-Fi changes state.

    Example:

      Wi-Fi connected
          ↓
      AlarmController
          ↓
      internetConnected = true
          ↓
      Normal health state
          ↓
      BLUE


      Wi-Fi disconnected
          ↓
      AlarmController
          ↓
      internetConnected = false
          ↓
      NET_DOWN
          ↓
      GREEN
  */

  if (wifiConnected != lastWifiConnected)
  {
    Serial.printf(
      "[WIFI] State changed: %s\n",
      wifiConnected ? "CONNECTED" : "DISCONNECTED"
    );


    /*
      Pass the new Wi-Fi state into the alarm controller.

      AlarmController is responsible for deciding what this
      means for the overall system health.

      main.cpp does NOT decide:

        "show green"

      It only passes information between modules.
    */

    alarmController.setInternetConnected(wifiConnected);


    /*
      Remember this state so we don't send the same update
      repeatedly.
    */

    lastWifiConnected = wifiConnected;
  }


  /*
    ----------------------------------------------------------
    4. UPDATE THE ALARM CONTROLLER
    ----------------------------------------------------------

    AlarmController::update() performs one non-blocking
    iteration of the alarm system.

    It handles:

      - Button input
      - Software power state
      - PIR input
      - Motion detection
      - PIR fault detection
      - Health-state evaluation
      - State transitions
      - LED output
      - Incognito mode
      - Future BLE commands
      - Future event generation
  */

  alarmController.update();


  /*
    ----------------------------------------------------------
    5. VERY SHORT YIELD
    ----------------------------------------------------------

    A 1 ms delay gives the ESP32 scheduler a chance to handle
    background system tasks.

    This is still effectively a non-blocking main loop.

    IMPORTANT:

    We do NOT use things like:

        delay(5000);

    for application logic.

    Long blocking delays would prevent the device from
    responding quickly to:

      - Buttons
      - PIR events
      - Wi-Fi
      - BLE
      - Server communication

    All application timing should continue using millis().
    ----------------------------------------------------------
  */

  delay(1);
}