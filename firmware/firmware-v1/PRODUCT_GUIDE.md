# Motion Alarm — Product User & Developer Guide

## Table of Contents

1. [Product Overview](#product-overview)
2. [User Guide](#user-guide)
3. [System Status Indicators](#system-status-indicators)
4. [Developer Setup](#developer-setup)
5. [Hardware Assembly](#hardware-assembly)
6. [Building and Uploading](#building-and-uploading)
7. [Troubleshooting](#troubleshooting)
8. [Known Limitations (Phase 1)](#known-limitations-phase-1)
9. [Roadmap](#roadmap)

---

## Product Overview

The Motion Alarm is a standalone ESP32-based motion detection system with health monitoring and visual feedback. It's designed for room-level intrusion detection, occupancy sensing, or any scenario requiring reliable, battery-efficient motion monitoring.

**Key Features (Phase 1):**
- Motion detection with visual alert (yellow LED)
- System health monitoring (RGB LED status indicator)
- Software power control (button press to standby)
- Incognito mode (disable LEDs while maintaining monitoring)
- Self-test on boot (white flash animation)
- Persistent settings (NVS storage)

**Target Use Cases:**
- Room security (doorway, entryway, office)
- Occupancy detection
- Alarm system expansion
- Development platform for IoT applications

---

## User Guide

### Normal Operation

#### Power On
1. Press the button briefly (< 5 seconds)
2. Device enters self-test mode (white LED flashes for 1 second)
3. System enters ready state (solid blue LED)

#### Detecting Motion
1. When motion is detected, the yellow LED illuminates
2. The system logs the event (via serial debug)
3. Yellow LED remains on for 5 seconds, then turns off
4. System returns to ready state (blue LED)

#### Power Off (Standby)
1. Press the button briefly (< 5 seconds)
2. All LEDs turn off
3. Device enters software standby (still draws minimal current, button remains active)
4. Button press wakes the device and re-enters boot sequence

#### Connection Reset (BLE Provisioning — Future)
1. Hold button for 5 seconds
2. Current implementation logs the request
3. Phase 2 will implement BLE provisioning mode
4. Device will advertise Bluetooth for WiFi credential entry

#### Incognito Mode (Future)
Currently controlled via firmware API:
```cpp
alarmController.setIncognito(true);   // Disable all LEDs
alarmController.setIncognito(false);  // Enable LEDs
```

Future phases will add BLE or mobile app control.

---

## System Status Indicators

### RGB Status LED (Pin 26 for blue, 25 for green, 13 for red)

| LED Color | Status | Meaning | Action Required |
|-----------|--------|---------|-----------------|
| **Flashing White** | BOOT | Self-test in progress | Wait 1 second |
| **Solid Blue** | NORMAL | System healthy and ready | None |
| **Solid Green** | INTERNET_FAULT | Network connection lost | Phase 2: WiFi will be added |
| **Solid Yellow** | PIR_FAULT | Motion sensor stuck or faulty | Check sensor; may need reset |
| **Solid Red** | BOTH_FAULT | PIR and network both down | Check sensor and network |
| **Off** | POWERED_OFF | Device in standby | Press button to wake |

### Yellow Motion Alert LED (Pin 27)

- **ON for 5 seconds** when motion is detected
- **OFF** otherwise
- Independent of system status (will turn on even during faults, except standby)

---

## Developer Setup

### Prerequisites

- **PlatformIO** (for building and uploading)
  - Install via VS Code extension or pip: `pip install platformio`
- **ESP32 DevKit-C v4** board
- **USB-to-Serial cable** for uploading
- **Python 3.x** (typically included with PlatformIO)

### Clone and Configure

```bash
# Clone the repository
git clone <repository-url>
cd motion-alarm/firmware/firmware-v1

# Open in VS Code with PlatformIO extension
code .

# Or use command line
pio project init --board esp32dev
```

### Environment Setup

**platformio.ini** is pre-configured:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

---

## Hardware Assembly

### BOM (Bill of Materials)

| Part | Qty | Notes |
|------|-----|-------|
| ESP32 DevKit-C v4 | 1 | Main microcontroller |
| PIR Motion Sensor | 1 | HC-SR501 or equivalent |
| RGB LED (common cathode) | 1 | 5mm or 10mm |
| Yellow LED | 1 | 5mm standard |
| Pushbutton (6mm) | 1 | Standard tactile switch |
| Resistor 220Ω | 4 | ¼W, limiting LED current |
| Battery/Power Supply | 1 | 5V USB or 4–5 AA batteries |
| Wires | — | 22 AWG hookup wire |

### GPIO Mapping

| Component | ESP32 GPIO |
|-----------|-----------|
| Pushbutton | 12 |
| PIR OUT | 14 |
| RGB Red | 13 |
| RGB Green | 25 |
| RGB Blue | 26 |
| Yellow Motion LED | 27 |

### Wiring Diagram

See `diagram.json` or the hardware folder for detailed schematics.

**Quick Reference:**
```
Button (Pin 12):  Active LOW (pull-down, GND when pressed)
PIR (Pin 14):     Active HIGH (outputs 5V when motion detected)
RGB LED:          Common cathode (GND shared, R/G/B via 220Ω resistors)
Yellow LED:       Standard LED (positive through 220Ω resistor)
```

---

## Building and Uploading

### Build (Compile Only)

```bash
# PlatformIO command line
pio run

# Or via VS Code: Press Ctrl+Alt+B (or use PlatformIO menu)
```

### Build and Upload

```bash
# PlatformIO command line
pio run -t upload

# Specify COM port (Windows)
pio run -t upload --upload-port COM3

# Specify serial port (Linux/Mac)
pio run -t upload --upload-port /dev/ttyUSB0
```

### Monitor Serial Output

```bash
# After upload, monitor debug output
pio device monitor --baud 115200

# Or via VS Code: Click PlatformIO → Serial Port Monitor
```

**Expected Boot Output:**
```
[BOOT] Motion Alarm starting...
[BOOT] Incognito: OFF
[STATE] -> 0 at 0 ms          (BOOT_SELFTEST)
[STATE] -> 1 at 1000 ms       (IDLE)
```

---

## Troubleshooting

### Device won't upload

**Problem:** "FAILED to execute tool esp32" or port not found

**Solutions:**
1. Check USB cable is connected and recognized
2. Install CH340 drivers if using DevKit-C (common on clones)
3. Specify explicit COM port: `pio run -t upload --upload-port COM3`
4. Try holding BOOT button while uploading (some boards require this)

### LEDs not responding

**Problem:** LEDs don't turn on or wrong colors

**Solutions:**
1. Verify GPIO pins match `Config.h`
2. Check resistor values (220Ω typical)
3. Confirm common cathode wiring (GND shared)
4. Test with a multimeter: LED should conduct in one direction only
5. Rebuild and upload latest firmware

### PIR sensor always showing fault

**Problem:** Yellow LED stays on, showing PIR_FAULT

**Solutions:**
1. Verify PIR sensor is connected to GPIO 14
2. Confirm PIR is powered (typically RED LED on sensor)
3. Allow PIR warm-up time (30 seconds) before moving in front of it
4. Check for obstructions (dust, tape) on sensor lens
5. If stuck, the sensor may be faulty — replace it

### Motion not detected

**Problem:** Yellow LED doesn't turn on when you move

**Solutions:**
1. Ensure device is powered ON (blue LED visible)
2. Wait 30 seconds for PIR warm-up after power on
3. Move more than 1–2 meters from sensor (typical PIR range)
4. Check PIR sensitivity dial (if adjustable)
5. Verify GPIO 14 wiring
6. Monitor serial output: should show `[EVENT] Motion detected`

### Button not responding

**Problem:** Power toggle and reset don't work

**Solutions:**
1. Verify GPIO 12 is connected
2. Confirm button is in pull-down configuration (GND when pressed)
3. Try pressing longer for long-press reset
4. Check for stuck button (physical issue)
5. Monitor serial: should show `[POWER]` or `[CONNECTION]` messages

---

## Known Limitations (Phase 1)

### Networking

- **No WiFi or BLE connectivity** — Network health defaults to "connected"
- **No cloud integration** — Events are logged locally only
- **No remote monitoring** — Cannot view status or control remotely

### Power Management

- **No low-power modes** — Device draws full current in standby
- **No sleep** — ESP32 scheduler remains active
- **No battery monitoring** — No voltage reporting or low-battery warning

### User Interface

- **No incognito app control** — Set via firmware API only
- **No settings UI** — Configuration is compile-time only
- **No reset confirmation** — Reset clears in next phase

### Sensor Limits

- **PIR range depends on sensor** — Typically 5–10 meters
- **No temperature compensation** — PIR sensitivity may drift
- **Single motion sensor** — Cannot triangulate or have redundancy

---

## Roadmap

### Phase 1 ✅ (Current)
- [x] Motion detection
- [x] Health monitoring (PIR fault detection)
- [x] RGB status LED
- [x] Software power control
- [x] Button control (power + reset placeholder)
- [x] Persistent settings (NVS)

### Phase 2 (Planned)
- [ ] BLE provisioning
- [ ] WiFi connectivity
- [ ] Network health monitoring
- [ ] Mobile app integration (basic)
- [ ] OTA update placeholder

### Phase 3 (Planned)
- [ ] MQTT client
- [ ] Cloud integration
- [ ] Event logging and history
- [ ] Remote monitoring

### Phase 4 (Planned)
- [ ] Hardware watchdog
- [ ] Deep sleep and low-power modes
- [ ] Offline event queue
- [ ] Advanced authentication

---

## API Reference (For Developers)

### AlarmController

```cpp
// Initialize system
void begin();

// Non-blocking main loop iteration
void update();

// Report network status (called by WiFi layer)
void setInternetConnected(bool connected);

// Enable/disable LED output
void setIncognito(bool enabled);

// Query current system state
SystemState getState() const;
```

### System States

```cpp
enum class SystemState {
    BOOT_SELFTEST,   // 0: Self-test animation
    IDLE,            // 1: Ready, no motion
    ALERT,           // 2: Motion detected
    PIR_FAULT,       // 3: PIR sensor fault
    NET_DOWN,        // 4: Network disconnected
    BOTH_FAULT,      // 5: Both PIR and network down
    POWERED_OFF      // 6: Software standby
};
```

### Example Integration (Phase 2)

```cpp
// In your WiFi connection handler:
void onWiFiConnected() {
    alarmController.setInternetConnected(true);
}

void onWiFiDisconnected() {
    alarmController.setInternetConnected(false);
}

// In your BLE provisioning handler:
void onBLECommandReceived(String cmd) {
    if (cmd == "INCOGNITO_ON") {
        alarmController.setIncognito(true);
    } else if (cmd == "INCOGNITO_OFF") {
        alarmController.setIncognito(false);
    }
}
```

---

## Support and Contributions

For issues, questions, or contributions:
1. Check the troubleshooting section above
2. Review COMPONENTS.md for technical details
3. Check `PRODUCT_SPEC.md` for design decisions
4. Submit issues with debug output from serial monitor

---

## License

(Add your license here)

---

## Version History

- **v1.0** (Phase 1) — Initial release with motion detection, health monitoring, and power control
- v2.0 (Phase 2) — BLE and WiFi (planned)
- v3.0 (Phase 3) — Cloud integration (planned)
- v4.0 (Phase 4) — Advanced features (planned)

