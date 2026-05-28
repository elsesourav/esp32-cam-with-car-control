#include "movement_parser.h"

#include "config.h"
#include <string.h>
#include <stdio.h>

namespace {
int clamp_int(int value, int min_value, int max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

float clamp_float(float value, float min_value, float max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

bool is_key_match(const char *cursor, const char *key) {
  while (*key && *cursor && *key == *cursor) {
    ++key;
    ++cursor;
  }
  return *key == '\0' && *cursor == '=';
}

bool parse_int(const char *text, int *out) {
  if (!text || !*text) {
    return false;
  }
  *out = atoi(text);
  return true;
}

bool parse_float(const char *text, float *out) {
  if (!text || !*text) {
    return false;
  }
  *out = atof(text);
  return true;
}

// Maps a direction string to a short serial code for the Arduino.
const char *dir_to_serial_code(const char *dir) {
  if (strcmp(dir, "STOP") == 0)           return "S";
  if (strcmp(dir, "FORWARD") == 0)        return "F";
  if (strcmp(dir, "BACKWARD") == 0)       return "B";
  if (strcmp(dir, "LEFT") == 0)           return "L";
  if (strcmp(dir, "RIGHT") == 0)          return "R";
  if (strcmp(dir, "FORWARD_LEFT") == 0)   return "FL";
  if (strcmp(dir, "FORWARD_RIGHT") == 0)  return "FR";
  if (strcmp(dir, "BACKWARD_LEFT") == 0)  return "BL";
  if (strcmp(dir, "BACKWARD_RIGHT") == 0) return "BR";
  return nullptr;
}

MovementCommand from_direction(const char *dir, int speed, int turn) {
  MovementCommand cmd;
  cmd.valid = true;

  if (strcmp(dir, "STOP") == 0) {
    cmd.left_speed = 0;
    cmd.right_speed = 0;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:S");
    return cmd;
  }
  if (strcmp(dir, "FORWARD") == 0) {
    cmd.left_speed = speed;
    cmd.right_speed = speed;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:F");
    return cmd;
  }
  if (strcmp(dir, "BACKWARD") == 0) {
    cmd.left_speed = -speed;
    cmd.right_speed = -speed;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:B");
    return cmd;
  }
  if (strcmp(dir, "LEFT") == 0) {
    cmd.left_speed = -speed;
    cmd.right_speed = speed;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:L");
    return cmd;
  }
  if (strcmp(dir, "RIGHT") == 0) {
    cmd.left_speed = speed;
    cmd.right_speed = -speed;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:R");
    return cmd;
  }
  if (strcmp(dir, "FORWARD_LEFT") == 0) {
    cmd.left_speed = speed - turn;
    cmd.right_speed = speed + turn;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:FL");
    return cmd;
  }
  if (strcmp(dir, "FORWARD_RIGHT") == 0) {
    cmd.left_speed = speed + turn;
    cmd.right_speed = speed - turn;
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:FR");
    return cmd;
  }
  if (strcmp(dir, "BACKWARD_LEFT") == 0) {
    cmd.left_speed = -(speed - turn);
    cmd.right_speed = -(speed + turn);
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:BL");
    return cmd;
  }
  if (strcmp(dir, "BACKWARD_RIGHT") == 0) {
    cmd.left_speed = -(speed + turn);
    cmd.right_speed = -(speed - turn);
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "MOVE:BR");
    return cmd;
  }

  cmd.valid = false;
  return cmd;
}
}  // namespace

bool movement_get_value(const char *payload, const char *key, char *out, size_t out_len) {
  if (!payload || !key || !out || out_len == 0) {
    return false;
  }

  const char *cursor = payload;
  while (*cursor) {
    if (is_key_match(cursor, key)) {
      const char *value = cursor + strlen(key) + 1;
      size_t i = 0;
      while (value[i] && value[i] != '&' && i + 1 < out_len) {
        out[i] = value[i];
        ++i;
      }
      out[i] = '\0';
      return true;
    }
    const char *next = strchr(cursor, '&');
    if (!next) {
      break;
    }
    cursor = next + 1;
  }

  return false;
}

MovementCommand movement_parse_command(const char *payload, const MovementSettings &settings) {
  MovementCommand cmd;
  cmd.valid = false;
  char mode[16] = {0};

  if (!movement_get_value(payload, "mode", mode, sizeof(mode))) {
    Serial.println("[PARSER Warning] Missing 'mode' in payload");
    return cmd;
  }

  if (strcmp(mode, "joystick") == 0) {
    char x_val[16] = {0};
    char y_val[16] = {0};
    if (!movement_get_value(payload, "x", x_val, sizeof(x_val)) ||
        !movement_get_value(payload, "y", y_val, sizeof(y_val))) {
      Serial.println("[PARSER Warning] Missing 'x' or 'y' for joystick mode");
      return cmd;
    }

    float x = 0, y = 0;
    parse_float(x_val, &x);
    parse_float(y_val, &y);
    
    Serial.print("[PARSER] Mode: JOYSTICK, Raw X: ");
    Serial.print(x);
    Serial.print(", Raw Y: ");
    Serial.println(y);

    float nx = clamp_float(x / 100.0f, -1.0f, 1.0f) * settings.turn;
    float ny = clamp_float(y / 100.0f, -1.0f, 1.0f) * settings.drive;

    float left = ny + nx;
    float right = ny - nx;

    int left_pwm = static_cast<int>(left * config::kMotorMaxPwm);
    int right_pwm = static_cast<int>(right * config::kMotorMaxPwm);

    cmd.left_speed = clamp_int(left_pwm, -config::kMotorMaxPwm, config::kMotorMaxPwm);
    cmd.right_speed = clamp_int(right_pwm, -config::kMotorMaxPwm, config::kMotorMaxPwm);

    // Format as differential command for the Arduino
    snprintf(cmd.serial_cmd, sizeof(cmd.serial_cmd), "DIFF:%d,%d",
             cmd.left_speed, cmd.right_speed);
             
    Serial.print("[PARSER] Parsed JOYSTICK to -> ");
    Serial.println(cmd.serial_cmd);
    
    cmd.valid = true;
    return cmd;
  }

  char dir[24] = {0};
  char speed_val[16] = {0};
  char turn_val[16] = {0};
  if (!movement_get_value(payload, "dir", dir, sizeof(dir))) {
    Serial.println("[PARSER Warning] Missing 'dir' for button mode");
    return cmd;
  }

  int speed = 180;
  if (movement_get_value(payload, "speed", speed_val, sizeof(speed_val))) {
    parse_int(speed_val, &speed);
  }

  int turn = 120;
  if (movement_get_value(payload, "turn", turn_val, sizeof(turn_val))) {
    parse_int(turn_val, &turn);
  }
  
  Serial.print("[PARSER] Mode: BUTTON, Dir: ");
  Serial.print(dir);
  Serial.print(", Speed: ");
  Serial.print(speed);
  Serial.print(", Turn: ");
  Serial.println(turn);

  cmd = from_direction(dir, speed, turn);
  
  if (cmd.valid) {
    Serial.print("[PARSER] Parsed BUTTON to -> ");
    Serial.println(cmd.serial_cmd);
  } else {
    Serial.println("[PARSER Warning] from_direction returned invalid command!");
  }
  
  return cmd;
}
