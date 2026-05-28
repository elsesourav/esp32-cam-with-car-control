#pragma once

#include <Arduino.h>
#include "camera_pins.h"

namespace config {
// --- WiFi ---
constexpr char kWifiSsid[] = "[[[[[[ SOURAV ]]]]]]";
constexpr char kWifiPass[] = "esp32esp";
constexpr char kHostname[] = "esp32-cam-car";

// --- HTTP / WebSocket ---
constexpr uint16_t kHttpPort = 80;
constexpr uint16_t kStreamPort = 81;
constexpr char kWsPath[] = "/ws";

// --- UART Bridge to Arduino Uno ---
// Uses ESP32 UART2 on GPIO 14 (TX) and GPIO 15 (RX).
// These pins are free on the ESP32-CAM header and do not conflict
// with the camera, PSRAM, or flashlight.
constexpr int kBridgeTxPin = 14;
constexpr int kBridgeRxPin = 15;
constexpr uint32_t kBridgeBaud = 38400;

// Motor command timeout — if no new command arrives within this time,
// the Arduino Uno will auto-stop motors. The ESP32 also re-sends STOP
// as a safety net.
constexpr uint32_t kCommandTimeoutMs = 400;

// Maximum PWM value sent in serial commands (used for clamping in
// movement parser).
constexpr uint8_t kMotorMaxPwm = 255;

// Telemetry broadcast interval (ms) — how often the ESP32 pushes
// telemetry updates to connected WebSocket clients.
constexpr uint32_t kTelemetryBroadcastMs = 200;

// --- Flashlight ---
// Uses the ESP32-CAM onboard flash LED (GPIO 4).
constexpr uint8_t kFlashlightPwmFreq = 5000;
constexpr uint8_t kFlashlightPwmResolution = 8;
#if defined(LED_GPIO_NUM)
constexpr int kFlashlightPin = LED_GPIO_NUM;
#else
constexpr int kFlashlightPin = -1;
#endif
}  // namespace config
