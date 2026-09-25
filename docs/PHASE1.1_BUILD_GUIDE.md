# Phase 1 — Build & Test Guide

## 1. Wiring
| Signal              | ESP32 pin | Through   | To                |
|----------------------|-----------|-----------|--------------------|
| RGB Red channel       | GPIO13    | 220Ω (r1) | RGB LED R pin      |
| RGB Green channel     | GPIO25    | 220Ω (r3) | RGB LED G pin      |
| RGB Blue channel      | GPIO26    | 220Ω (r4) | RGB LED B pin      |
| RGB common            | GND       | —         | RGB LED COM pin    |
| Yellow "motion" LED   | GPIO27    | 220Ω (r2) | Yellow LED anode   |
| Pushbutton            | GPIO12    | —         | Button, other leg to GND |
| PIR OUT               | GPIO14    | —         | PIR OUT pin        |
| PIR VCC / GND         | 3V3 / GND | —         | PIR VCC / GND      |

This matches `hardware/diagram.json` — open it directly in Wokwi
(wokwi.com → New Project → paste/import) to simulate before touching real
hardware, since your original sketch was built there.

**If your RGB LED is common anode instead of common cathode:** flip the logic
in `setRGB()` (write `LOW` for "on" instead of `HIGH`), and wire COM to 3V3
instead of GND.

## 2. Flashing
1. Arduino IDE → Boards Manager → install "esp32" (Espressif) if not already
   installed.
2. Board: "ESP32 Dev Module" (or your specific DevKit-C variant).
3. Open `firmware/motion_alarm_phase1/motion_alarm_phase1.ino`.
4. No external libraries needed — `Preferences.h` ships with the ESP32 core.
5. Select the correct COM port, upload.
6. Open Serial Monitor at 115200 baud to watch state transitions and events.

## 3. Test checklist (this is your "ready for Phase 2" signal)
- [ ] On power-up, RGB flashes white briefly, then goes solid blue.
- [ ] Waving in front of the PIR triggers `[EVENT] Motion detected` in serial,
      the yellow LED lights for ~5s, then turns off — RGB stays blue the
      whole time.
- [ ] Holding the button 3s toggles incognito — all LEDs go dark regardless
      of state, and serial confirms `[MODE] Incognito ON/OFF`.
- [ ] Power-cycle the board while incognito is ON — it should boot back into
      incognito mode (confirms NVS persistence).
- [ ] You can explain, without opening the code, exactly which state the
      device is in and why, just from the LED + serial log.

## 4. Tuning notes
- `PIR_STUCK_FAULT_MS` (currently 120000 ms / 2 min) is a starting guess.
  Some PIR modules retrigger and hold OUT high for a while during genuine
  continuous motion — walk in front of yours for a sustained period and
  watch whether it stays HIGH long enough to false-trigger `PIR_FAULT`.
  Raise the threshold if so.
- `PIR_WARMUP_MS` (30s) ignores PIR output right after boot, since most
  modules output garbage while settling. If yours is noisy longer than that,
  increase it.

## 5. Known limitations at this phase
- Short button press is logged but does nothing yet — reserved for a future
  "silence/acknowledge" action once there's a backend to report to.
- `STATE_NET_DOWN` / solid green cannot be reached yet — no network code
  exists. This is expected, not a bug.
- No debounce edge case handling for extremely noisy PIR wiring (breadboard
  jumper noise) — if you see spurious edges on real hardware, add a hardware
  decoupling cap across the PIR's power pins first before touching firmware.
