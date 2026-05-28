#include "movement_parser.h"
#include "pwm_controller.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>

namespace {
int g_base_speed = 180;
float g_turn_sensitivity = 1.0f;
float g_drive_sensitivity = 1.0f;

// Current movement state name for telemetry.
const char *g_state = "STOP";

void apply_move(const char *dir) {
  int spd = g_base_speed;
  int turn_spd = (int)(spd * 0.65f);  // Turning bias
  int tgt_l = 0, tgt_r = 0;

  if (strcmp(dir, "S") == 0) {
    g_state = "STOP";
    tgt_l = 0; tgt_r = 0;
  } else if (strcmp(dir, "F") == 0) {
    g_state = "FORWARD";
    tgt_l = spd; tgt_r = spd;
  } else if (strcmp(dir, "B") == 0) {
    g_state = "BACKWARD";
    tgt_l = -spd; tgt_r = -spd;
  } else if (strcmp(dir, "L") == 0) {
    g_state = "LEFT";
    tgt_l = -spd; tgt_r = spd;
  } else if (strcmp(dir, "R") == 0) {
    g_state = "RIGHT";
    tgt_l = spd; tgt_r = -spd;
  } else if (strcmp(dir, "FL") == 0) {
    g_state = "FORWARD_LEFT";
    tgt_l = spd - turn_spd; tgt_r = spd + turn_spd;
  } else if (strcmp(dir, "FR") == 0) {
    g_state = "FORWARD_RIGHT";
    tgt_l = spd + turn_spd; tgt_r = spd - turn_spd;
  } else if (strcmp(dir, "BL") == 0) {
    g_state = "BACKWARD_LEFT";
    tgt_l = -(spd - turn_spd); tgt_r = -(spd + turn_spd);
  } else if (strcmp(dir, "BR") == 0) {
    g_state = "BACKWARD_RIGHT";
    tgt_l = -(spd + turn_spd); tgt_r = -(spd - turn_spd);
  }

  Serial.print(F("[Move] "));
  Serial.print(g_state);
  Serial.print(F(" | L: "));
  Serial.print(tgt_l);
  Serial.print(F(" R: "));
  Serial.println(tgt_r);

  pwm_controller_set_target(tgt_l, tgt_r);
}

void apply_diff(const char *args) {
  // Format: "left,right" e.g. "120,-120"
  char buf[32];
  strncpy(buf, args, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char *comma = strchr(buf, ',');
  if (!comma) return;
  *comma = '\0';

  int left = atoi(buf);
  int right = atoi(comma + 1);

  if (left == 0 && right == 0) {
    g_state = "STOP";
  } else {
    g_state = "JOYSTICK";
  }

  Serial.print(F("[Diff] "));
  Serial.print(g_state);
  Serial.print(F(" | L: "));
  Serial.print(left);
  Serial.print(F(" R: "));
  Serial.println(right);

  pwm_controller_set_target(left, right);
}
}  // namespace

bool movement_parser_handle(const char *line) {
  if (strncmp(line, "MOVE:", 5) == 0) {
    apply_move(line + 5);
    return true;
  }
  if (strncmp(line, "DIFF:", 5) == 0) {
    apply_diff(line + 5);
    return true;
  }
  if (strncmp(line, "SPD:", 4) == 0) {
    g_base_speed = atoi(line + 4);
    if (g_base_speed < 0) g_base_speed = 0;
    if (g_base_speed > MOTOR_MAX_PWM) g_base_speed = MOTOR_MAX_PWM;
    Serial.print(F("[Parser] Speed: "));
    Serial.println(g_base_speed);
    return true;
  }
  if (strncmp(line, "TURN:", 5) == 0) {
    g_turn_sensitivity = atof(line + 5);
    Serial.print(F("[Parser] Turn: "));
    Serial.println(g_turn_sensitivity);
    return true;
  }
  if (strncmp(line, "DRIVE:", 6) == 0) {
    g_drive_sensitivity = atof(line + 6);
    Serial.print(F("[Parser] Drive: "));
    Serial.println(g_drive_sensitivity);
    return true;
  }

  return false;
}

int movement_parser_get_speed() { return g_base_speed; }
float movement_parser_get_turn() { return g_turn_sensitivity; }
float movement_parser_get_drive() { return g_drive_sensitivity; }
