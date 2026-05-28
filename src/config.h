#pragma once

#include <Arduino.h>
#include "camera_pins.h"

namespace config {
constexpr char kWifiSsid[] = "[[[[[[ SOURAV ]]]]]]";
constexpr char kWifiPass[] = "esp32esp";
constexpr char kHostname[] = "esp32-cam-car";

constexpr uint16_t kHttpPort = 80;
constexpr uint16_t kStreamPort = 81;
constexpr char kWsPath[] = "/ws";

constexpr uint32_t kMotorPwmFreq = 20000;
constexpr uint8_t kMotorPwmResolution = 8;
constexpr uint8_t kMotorMaxPwm = 255;
constexpr uint32_t kCommandTimeoutMs = 400;

// Motor control pins: 4 motors, 1 direction pin + 1 PWM pin each.
//
// Selected pins are chosen carefully to avoid crashing the ESP32-CAM (0 and 16 are avoided)
// and to avoid turning on the flashlight (4 is avoided).

// Motor A (front-left)
constexpr int kMotorAFwd = 15;   // GPIO15
constexpr int kMotorAPwm = 13;   // GPIO13

// Motor B (front-right)
constexpr int kMotorBFwd = 12;   // GPIO12
constexpr int kMotorBPwm = 14;   // GPIO14

// Motor C (rear-left)
constexpr int kMotorCFwd = 2;    // GPIO2
constexpr int kMotorCPwm = 3;    // GPIO3 (U0R)

// Motor D (rear-right)
constexpr int kMotorDFwd = 1;    // GPIO1 (U0T)
constexpr int kMotorDPwm = 33;   // GPIO33 (Requires soldering to the small red LED on the back)

// Flashlight LED uses the ESP32-CAM onboard LED unless overridden.
constexpr uint8_t kFlashlightPwmFreq = 5000;
constexpr uint8_t kFlashlightPwmResolution = 8;
#if defined(LED_GPIO_NUM)
constexpr int kFlashlightPin = LED_GPIO_NUM; // Restored GPIO 4 for the flashlight
#else
constexpr int kFlashlightPin = -1;
#endif
}  // namespace config
