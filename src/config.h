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

// Left driver (L298N) pins.
constexpr int kLeftIn1 = 12;
constexpr int kLeftIn2 = 13;
constexpr int kLeftEna = 14;  // PWM
constexpr int kLeftIn3 = 27;
constexpr int kLeftIn4 = 26;
constexpr int kLeftEnb = 25;  // PWM

// Right driver (L298N) pins.
constexpr int kRightIn1 = 33;
constexpr int kRightIn2 = 32;
constexpr int kRightEna = 15;  // PWM
constexpr int kRightIn3 = 4;
constexpr int kRightIn4 = 2;
constexpr int kRightEnb = 5;   // PWM

// Flashlight LED uses the ESP32-CAM onboard LED unless overridden.
constexpr uint8_t kFlashlightPwmFreq = 5000;
constexpr uint8_t kFlashlightPwmResolution = 8;
#if defined(LED_GPIO_NUM)
constexpr int kFlashlightPin = LED_GPIO_NUM;
#else
constexpr int kFlashlightPin = -1;
#endif
}  // namespace config
