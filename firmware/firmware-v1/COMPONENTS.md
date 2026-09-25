# Motion Alarm — Component Architecture

## Overview

This document describes the software components that make up the Motion Alarm firmware. The system uses a modular architecture centered around the **AlarmController**, which orchestrates the behavior of sensor inputs, button controls, and visual feedback.

---

## Core Architecture

```
                    main.cpp
                       |
                       v
                AlarmController
                  /    |     \
                 /     |      \
                v      v       v
             Button   PIR    StatusLED
                              |
                              v
                         RGB + Yellow

                    SettingsStore
                         |
                         v
                         NVS (Preferences)
```

---

## Component Reference

### AlarmController

**Purpose:** The central state machine and system orchestrator.

**Responsibilities:**
- Manages all system states (BOOT, IDLE, ALERT, PIR_FAULT, NET_DOWN, BOTH_FAULT, POWERED_OFF)
- Coordinates input from button and PIR sensor
- Evaluates health (PIR fault detection + network connectivity)
- Drives transitions between states based on health and events
- Commands LED output to reflect current system state
- Manages software power control (standby mode)

**Key Methods:**
- `begin()` — Initialize all subsystems and enter BOOT_SELFTEST
- `update()` — Non-blocking main loop iteration
- `setInternetConnected(bool)` — Report network status (called by WiFi/BLE layer)
- `setIncognito(bool)` — Enable/disable all LED output
- `getState()` — Query current system state

**State Machine:**

| State | Entry Condition | Exit Condition |
|-------|-----------------|----------------|
| BOOT_SELFTEST | Power ON or button press in standby | After 1 second timeout |
| IDLE | Boot complete, no faults, no motion | Motion detected or fault develops |
| ALERT | Motion edge detected | 5-second timeout or fault develops |
| PIR_FAULT | PIR stuck HIGH detected | PIR recovers and no network fault |
| NET_DOWN | Network connectivity lost | Network recovers and no PIR fault |
| BOTH_FAULT | PIR fault AND network down | At least one fault clears |
| POWERED_OFF | User short-press button | User short-press button (re-enters BOOT) |

**Health-State Evaluation:**

Health is evaluated **every loop iteration** in all active states (IDLE, ALERT, and fault states). This ensures:
- Network faults are detected immediately during normal operation
- PIR faults are continuously monitored
- Fault recovery is instantaneous when conditions clear
- State transitions are driven by the comprehensive `updateHealthState()` method

```cpp
// Evaluated every iteration, including IDLE and ALERT
updateHealthState(pirFault);

// Rules:
// 1. If both PIR and network fault → BOTH_FAULT
// 2. If only PIR fault → PIR_FAULT
// 3. If only network fault → NET_DOWN
// 4. If no faults → IDLE
```

---

### Button

**Purpose:** Handle user input (power and reset control).

**Responsibilities:**
- Read button GPIO (active LOW)
- Debounce input (40 ms)
- Detect short press (button released within debounce window)
- Detect long press (held for 5 seconds)

**GPIO:** Pin 12 (active LOW)

**Key Methods:**
- `begin(pin)` — Initialize button input
- `update()` — Non-blocking input processing
- `wasShortPressed()` — Returns true once per short press
- `wasLongPressed()` — Returns true once per 5-second press

**Behavior:**
- **Short Press (< 5s):** Toggle software power ON/OFF
- **Long Press (≥ 5s):** Request connection reset (placeholder for BLE provisioning)

---

### PirSensor

**Purpose:** Detect motion and monitor PIR sensor health.

**Responsibilities:**
- Read PIR GPIO (active HIGH)
- Detect motion edge (LOW → HIGH transition)
- Monitor for stuck-HIGH fault condition
- Ignore sensor noise during warm-up period (first 30 seconds)

**GPIO:** Pin 14 (active HIGH)

**Key Methods:**
- `begin(pin)` — Initialize PIR input
- `update()` — Non-blocking edge detection
- `motionDetected()` — Returns true if motion edge was detected
- `hasFault()` — Returns true if PIR is stuck HIGH for > 120 seconds

**Fault Detection Logic:**

```cpp
// PIR is considered faulty if:
// 1. Warm-up period (30s) has completed
// 2. PIR output has been HIGH for ≥ 120 seconds
//
// This detects sensor malfunction or obstruction
// (e.g., sensor covered by dust or debris)
```

**Timing:**
- PIR Warm-up: 30 seconds (sensor self-stabilization)
- Stuck-HIGH Fault Threshold: 120 seconds

---

### StatusLED

**Purpose:** Provide visual feedback of system health and events.

**Responsibilities:**
- Control RGB LED (red, green, blue channels on GPIO 13, 25, 26)
- Control yellow motion-alert LED (GPIO 27)
- Manage boot self-test animation (white flash)
- Apply incognito mode (disable all LED output)
- Apply power state (disable LEDs during standby)

**GPIO Mapping:**
| Component | Pin |
|-----------|-----|
| RGB Red | 13 |
| RGB Green | 25 |
| RGB Blue | 26 |
| Yellow Motion LED | 27 |

**LED Status Mapping:**

| Status | RGB | Meaning |
|--------|-----|---------|
| BOOT | White flash (150 ms) | Self-test in progress |
| NORMAL | Blue | System healthy, ready |
| INTERNET_FAULT | Green | Network disconnected |
| PIR_FAULT | Yellow (R+G) | PIR sensor fault |
| BOTH_FAULT | Red | Both PIR and network faults |
| OFF | Dark | Powered off or incognito |

**Motion Alert LED:**
- Turns ON for 5 seconds when motion is detected (ALERT state)
- Remains OFF during faults, boot, and standby
- Independent of incognito mode during active monitoring

**Key Methods:**
- `begin(r, g, b, yellow)` — Initialize LED pins
- `setStatus(LEDStatus)` — Set system health indicator color
- `setMotionAlert(bool)` — Enable/disable motion event LED
- `setIncognito(bool)` — Hide all visual feedback
- `setPowered(bool)` — Disable LEDs during standby
- `update()` — Refresh LED output

---

### SettingsStore

**Purpose:** Persist configuration across power cycles using ESP32 NVS.

**Responsibilities:**
- Initialize ESP32 Preferences API
- Read and write user settings
- Currently stores: **Incognito mode state**
- Future expansion: WiFi credentials, alarm thresholds, timezone

**Key Methods:**
- `begin()` — Open NVS namespace
- `getIncognito()` — Read incognito mode state
- `setIncognito(bool)` — Save incognito mode state

**Storage Backend:** ESP32 NVS (Non-Volatile Storage, internal flash)

**Namespace:** `"motion-alarm"` (reserved for this application)

---

## Data Flow

### Motion Detection Flow

```
1. PirSensor::update()
   └─ Reads GPIO 14, detects LOW→HIGH edge
   
2. AlarmController::handleAlarmLogic()
   └─ Calls pir.motionDetected()
   └─ If IDLE and motion → enterState(ALERT)
   
3. AlarmController::updateOutputs()
   └─ Sets LED status NORMAL, motion alert ON
   
4. StatusLED::update()
   └─ Turns ON RGB (blue) + yellow (GPIO 27)
   
5. After 5 seconds (ALERT_DURATION_MS)
   └─ enterState(IDLE)
   └─ Motion alert LED turns OFF
```

### Fault Detection Flow

```
1. PIR stuck HIGH for > 120 seconds
   └─ pir.hasFault() returns true
   
2. AlarmController::updateHealthState()
   └─ Checks: pirFault && !internetConnected
   └─ Enters appropriate fault state
   
3. AlarmController::updateOutputs()
   └─ Sets LED status (yellow/red/green)
   
4. When fault clears (PIR recovers or network reconnects)
   └─ updateHealthState() re-evaluates
   └─ Returns to IDLE (if both faults clear)
```

### Button Input Flow

```
1. Button::update()
   └─ Reads GPIO 12, debounces (40 ms)
   └─ Detects short or long press
   
2. AlarmController::handleButton()
   └─ If short press: togglePower()
   └─ If long press: resetConnection()
   
3. Short Press: togglePower()
   └─ Flip poweredOn flag
   └─ If OFF: enterState(POWERED_OFF), kill LEDs
   └─ If ON: enterState(BOOT_SELFTEST), resume operation
   
4. Long Press: resetConnection()
   └─ Placeholder: Will enter BLE provisioning in Phase 2
```

---

## Configuration Constants

All timing and pin configurations are defined in `include/Config.h`:

```cpp
// GPIO Pins
constexpr uint8_t PIN_BTN          = 12;
constexpr uint8_t PIN_PIR          = 14;
constexpr uint8_t PIN_LED_R        = 13;
constexpr uint8_t PIN_LED_G        = 25;
constexpr uint8_t PIN_LED_B        = 26;
constexpr uint8_t PIN_LED_YELLOW   = 27;

// Timing (milliseconds)
constexpr unsigned long PIR_WARMUP_MS       = 30000;   // 30 seconds
constexpr unsigned long PIR_STUCK_FAULT_MS  = 120000;  // 120 seconds
constexpr unsigned long BTN_LONGPRESS_MS    = 5000;    // 5 seconds
constexpr unsigned long DEBOUNCE_MS         = 40;      // 40 milliseconds
constexpr unsigned long ALERT_DURATION_MS   = 5000;    // 5 seconds
constexpr unsigned long BOOT_SELFTEST_MS    = 1000;    // 1 second
constexpr unsigned long WHITE_FLASH_MS      = 150;     // 150 milliseconds
```

---

## Non-Blocking Design

All components are designed for non-blocking operation:

- **No `delay()` in main loop** — Only 1 ms yield for ESP32 scheduler
- **State machines** — Each `update()` performs one iteration
- **Timing** — Uses `millis()` for state duration tracking
- **Ready for async I/O** — BLE, WiFi, and MQTT will integrate seamlessly

The firmware can respond to button input, motion detection, and network events within milliseconds.

---

## Future Expansion Points

**Phase 2 — Networking:**
- BLE provisioning module (GPIO-agnostic)
- WiFi connectivity monitoring
- Network reconnection logic

**Phase 3 — Cloud:**
- MQTT client
- Event reporter
- Server communication

**Phase 4 — Robustness:**
- Hardware watchdog
- OTA updates
- Event queue and offline support
- Low-power modes (deep sleep)

The core state machine and component API are designed to support these expansions without refactoring.

---

## Testing Checklist

- [ ] Boot animation (white flash) plays for 1 second
- [ ] System enters IDLE after boot
- [ ] Motion triggers ALERT state and yellow LED illuminates
- [ ] ALERT expires after 5 seconds
- [ ] Short button press toggles power (enters POWERED_OFF, LEDs off)
- [ ] Short button press again re-boots system
- [ ] Network status change via `setInternetConnected()` updates LED
- [ ] PIR stuck-HIGH after 120 seconds triggers PIR_FAULT and yellow LED
- [ ] Both faults present → red LED
- [ ] Incognito mode disables all LEDs but preserves monitoring
- [ ] Long button press logs "reset requested" (Phase 2 integration)
- [ ] Alarm recovers from faults when conditions clear
