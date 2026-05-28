#pragma once

#include <Arduino.h>

struct MovementSettings {
  float drive = 1.0f;
  float turn = 1.0f;
};

struct MovementCommand {
  int left_speed = 0;
  int right_speed = 0;
  bool valid = false;
  // Formatted serial command to send to Arduino Uno.
  // e.g. "DIFF:120,-120" or "MOVE:F"
  char serial_cmd[32] = {0};
};

bool movement_get_value(const char *payload, const char *key, char *out, size_t out_len);

MovementCommand movement_parse_command(const char *payload, const MovementSettings &settings);
