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

// --- Motor Control (L298N) ---
// Left motor (Motor A)
constexpr int kMotorLeftIn1  = 12;  // GPIO 12 → L298N IN1 (forward, must be LOW at boot)
constexpr int kMotorLeftIn2  = 13;  // GPIO 13 → L298N IN2 (backward)

// Right motor (Motor B)
constexpr int kMotorRightIn3 = 2;   // GPIO 2  → L298N IN3 (forward, must be LOW for flash)
constexpr int kMotorRightIn4 = 3;   // GPIO 3  → L298N IN4 (backward, RX pin - UNPLUG TO FLASH!)

// Motor PWM configuration
constexpr uint32_t kMotorPwmFreq       = 1000;  // 1 kHz — good for DC motors
constexpr uint8_t  kMotorPwmResolution = 8;      // 8-bit = 0–255
constexpr uint8_t  kMotorMaxPwm        = 255;

// PWM smoothing — ramp speed (how fast motors accelerate/decelerate)
constexpr uint8_t  kMotorRampStep      = 15;     // PWM units per tick
constexpr uint32_t kMotorRampIntervalMs = 20;    // tick interval (ms)

// Motor command timeout — if no new command arrives within this time,
// motors auto-stop as a safety net.
constexpr uint32_t kCommandTimeoutMs = 400;

// --- MPU6050 (I2C) ---
constexpr int kMpuSdaPin  = 14;  // GPIO 14
constexpr int kMpuSclPin  = 15;  // GPIO 15 (Must be high at boot - pulled up by MPU)
constexpr uint8_t kMpuAddr = 0x68;
constexpr uint32_t kMpuReadIntervalMs = 100;  // Read sensor every 100ms

// Telemetry broadcast interval (ms)
constexpr uint32_t kTelemetryBroadcastMs = 200;

// Camera stream frame pacing (ms between frames).
// 250ms = 4 FPS — smoother stream now that resolution is reduced
constexpr uint32_t kStreamFrameDelayMs = 250;

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
