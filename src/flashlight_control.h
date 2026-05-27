#pragma once

#include <Arduino.h>

void flashlight_init();
void flashlight_set(bool on);
void flashlight_set_level(uint8_t level);
uint8_t flashlight_get_level();
bool flashlight_is_on();
