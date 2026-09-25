# Motion Alarm — Product Spec (v0.1, Phase 1 scope)

## 1. Summary
A standalone motion-detection alarm device (ESP32-based) that reports its own
health via an RGB status LED and a dedicated motion-event LED. Designed to
grow into a networked, app-provisioned product without a firmware rewrite —
each build phase below only adds capability, it doesn't restructure what came
before it.

## 2. Target user
Someone who wants a self-contained motion sensor for a room, doorway, or
entryway, monitorable at a glance (LED color) and eventually remotely
(phone app + cloud, later phases).

## 3. Hardware (Phase 1 BOM)
| Part                  | Qty | Notes                                  |
|------------------------|-----|-----------------------------------------|
| ESP32 DevKit-C v4       | 1   | Main controller                         |
| PIR motion sensor       | 1   | Digital OUT, active HIGH on motion      |
| RGB LED (common cathode)| 1   | Status indicator                        |
| LED, yellow             | 1   | Motion-event indicator                  |
| Pushbutton (6mm)        | 1   | Mode control (incognito toggle for now) |
| Resistor, 220Ω          | 4   | One per LED channel (R, G, B, yellow)   |

See `hardware/diagram.json` (Wokwi format) for exact wiring.

## 4. Status LED behavior
| Color / pattern     | Meaning                                  | Phase wired in |
|----------------------|--------------------------------------------|----------------|
| Fast white flash     | Booting / self-test                        | 1              |
| Solid blue           | Idle or alert, system healthy              | 1              |
| Blinking red         | PIR fault (output stuck HIGH too long)     | 1              |
| Solid green          | No internet connection                     | 2 (placeholder now) |
| All off (any state)  | Incognito mode is on                       | 1              |

Yellow LED lights only while `STATE_ALERT` is active (a few seconds after a
motion edge), independent of the RGB status color.

## 5. Incognito mode
- Purpose: let the user physically disable all visible light output (e.g.
  overnight, or in a room where a blinking LED is unwanted) without touching
  the phone app or losing the underlying alarm/sensing logic.
- Toggle: hold the pushbutton for 3 seconds.
- Persistence: stored in NVS (`Preferences`), survives power loss/reboot.
- Scope: gates all LED writes at a single choke point (`setRGB()` /
  `updateYellowLED()`), so no other logic needs to know incognito exists.

## 6. State machine (Phase 1)
`BOOT_SELFTEST → IDLE ⇄ ALERT`, with `PIR_FAULT` reachable from either and
returning to `IDLE` once the fault clears. `NET_DOWN` exists in the enum as a
placeholder for Phase 2 but is not yet reachable — this is intentional, so
adding Wi-Fi/MQTT later doesn't require reshaping the state machine.

## 7. Roadmap (for context, not all in scope yet)
1. **State machine** — done, this spec's scope.
2. **BLE provisioning / non-blocking networking** — in progress separately.
3. Wi-Fi + MQTT — not started.
4. Event IDs, offline queue, HTTPS, auth, watchdog, OTA — later, per the
   original roadmap document.

## 8. Explicitly out of scope for v1
- Any network connectivity (Wi-Fi, BLE app control, MQTT).
- Cloud backend / event storage.
- OTA updates.
- Enclosure / mechanical design.

## 9. Open questions
- Final PIR "stuck fault" threshold — needs tuning against the real sensor's
  retrigger behavior (see build guide, section on tuning).
- Whether incognito mode should also be toggleable remotely once BLE/app
  control exists (natural extension, not yet decided).
