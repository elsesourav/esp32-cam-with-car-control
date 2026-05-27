#include "flashlight_control.h"

#include "config.h"
#include "esp32-hal-ledc.h"

namespace {
uint8_t g_level = 0;
bool g_enabled = false;
}

void flashlight_init() {
  if (config::kFlashlightPin < 0) {
    return;
  }
  ledcAttach(config::kFlashlightPin, config::kFlashlightPwmFreq, config::kFlashlightPwmResolution);
  flashlight_set_level(0);
  flashlight_set(false);
}

void flashlight_set(bool on) {
  if (config::kFlashlightPin < 0) {
    g_enabled = false;
    return;
  }
  
  if (g_enabled != on) {
    g_enabled = on;
    Serial.print("Flashlight Action: Toggled ");
    Serial.println(on ? "ON" : "OFF");
  }
  
  ledcWrite(config::kFlashlightPin, on ? g_level : 0);
}

void flashlight_set_level(uint8_t level) {
  if (config::kFlashlightPin < 0) {
    return;
  }
  
  if (g_level != level) {
    g_level = level;
    Serial.print("Flashlight Action: Brightness set to ");
    Serial.println(level);
  }
  
  if (g_enabled) {
    ledcWrite(config::kFlashlightPin, g_level);
  }
}

uint8_t flashlight_get_level() {
  return g_level;
}

bool flashlight_is_on() {
  return g_enabled;
}
