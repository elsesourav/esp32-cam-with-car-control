#include "movement_parser.h"

#include "config.h"
#include <string.h>

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

MovementCommand from_direction(const char *dir, int speed, int turn) {
  MovementCommand cmd;
  cmd.valid = true;

  if (strcmp(dir, "STOP") == 0) {
    cmd.left_speed = 0;
    cmd.right_speed = 0;
    return cmd;
  }
  if (strcmp(dir, "FORWARD") == 0) {
    cmd.left_speed = speed;
    cmd.right_speed = speed;
    return cmd;
  }
  if (strcmp(dir, "BACKWARD") == 0) {
    cmd.left_speed = -speed;
    cmd.right_speed = -speed;
    return cmd;
  }
  if (strcmp(dir, "LEFT") == 0) {
    cmd.left_speed = -speed;
    cmd.right_speed = speed;
    return cmd;
  }
  if (strcmp(dir, "RIGHT") == 0) {
    cmd.left_speed = speed;
    cmd.right_speed = -speed;
    return cmd;
  }
  if (strcmp(dir, "FORWARD_LEFT") == 0) {
    cmd.left_speed = speed - turn;
    cmd.right_speed = speed + turn;
    return cmd;
  }
  if (strcmp(dir, "FORWARD_RIGHT") == 0) {
    cmd.left_speed = speed + turn;
    cmd.right_speed = speed - turn;
    return cmd;
  }
  if (strcmp(dir, "BACKWARD_LEFT") == 0) {
    cmd.left_speed = -(speed - turn);
    cmd.right_speed = -(speed + turn);
    return cmd;
  }
  if (strcmp(dir, "BACKWARD_RIGHT") == 0) {
    cmd.left_speed = -(speed + turn);
    cmd.right_speed = -(speed - turn);
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

  char type[16] = {0};
  if (!movement_get_value(payload, "type", type, sizeof(type))) {
    return cmd;
  }

  if (strcmp(type, "move") != 0) {
    return cmd;
  }

  char mode[16] = {0};
  movement_get_value(payload, "mode", mode, sizeof(mode));

  if (strcmp(mode, "joystick") == 0) {
    char x_val[16] = {0};
    char y_val[16] = {0};
    if (!movement_get_value(payload, "x", x_val, sizeof(x_val)) ||
        !movement_get_value(payload, "y", y_val, sizeof(y_val))) {
      return cmd;
    }

    int x = 0;
    int y = 0;
    if (!parse_int(x_val, &x) || !parse_int(y_val, &y)) {
      return cmd;
    }

    float nx = clamp_float(x / 100.0f, -1.0f, 1.0f) * settings.turn;
    float ny = clamp_float(y / 100.0f, -1.0f, 1.0f) * settings.drive;

    float left = ny + nx;
    float right = ny - nx;

    int left_pwm = static_cast<int>(left * config::kMotorMaxPwm);
    int right_pwm = static_cast<int>(right * config::kMotorMaxPwm);

    cmd.left_speed = clamp_int(left_pwm, -config::kMotorMaxPwm, config::kMotorMaxPwm);
    cmd.right_speed = clamp_int(right_pwm, -config::kMotorMaxPwm, config::kMotorMaxPwm);
    cmd.valid = true;
    return cmd;
  }

  char dir[24] = {0};
  char speed_val[16] = {0};
  char turn_val[16] = {0};
  if (!movement_get_value(payload, "dir", dir, sizeof(dir))) {
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

  speed = clamp_int(speed, 0, config::kMotorMaxPwm);
  turn = clamp_int(turn, 0, config::kMotorMaxPwm);

  return from_direction(dir, speed, turn);
}
