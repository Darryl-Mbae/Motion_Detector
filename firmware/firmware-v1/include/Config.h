#pragma once

#include <Arduino.h>

// ---------- Hardware pins ----------

constexpr uint8_t PIN_BTN        = 12;
constexpr uint8_t PIN_PIR        = 14;

constexpr uint8_t PIN_LED_R      = 13;
constexpr uint8_t PIN_LED_G      = 25;
constexpr uint8_t PIN_LED_B      = 26;

constexpr uint8_t PIN_LED_YELLOW = 27;

// ---------- Timing ----------

constexpr unsigned long PIR_WARMUP_MS      = 30000;
constexpr unsigned long PIR_STUCK_FAULT_MS = 120000;

constexpr unsigned long BTN_LONGPRESS_MS   = 5000;
constexpr unsigned long DEBOUNCE_MS        = 40;

constexpr unsigned long ALERT_DURATION_MS  = 5000;
constexpr unsigned long BOOT_SELFTEST_MS   = 1000;
constexpr unsigned long WHITE_FLASH_MS     = 150;