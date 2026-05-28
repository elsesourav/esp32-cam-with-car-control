#include "pwm_controller.h"
#include "config.h"

namespace {
int g_target_left = 0;
int g_target_right = 0;
int g_current_left = 0;
int g_current_right = 0;
unsigned long g_last_ramp_ms = 0;

// Ramp a current value towards a target value by step amount.
int ramp_towards(int current, int target, int step) {
  if (current < target) {
    current += step;
    if (current > target) current = target;
  } else if (current > target) {
    current -= step;
    if (current < target) current = target;
  }
  return current;
}
}  // namespace

void pwm_controller_init() {
  g_target_left = 0;
  g_target_right = 0;
  g_current_left = 0;
  g_current_right = 0;
  g_last_ramp_ms = millis();
}

void pwm_controller_set_target(int left_target, int right_target) {
  // Clamp targets.
  if (left_target > MOTOR_MAX_PWM) left_target = MOTOR_MAX_PWM;
  if (left_target < -MOTOR_MAX_PWM) left_target = -MOTOR_MAX_PWM;
  if (right_target > MOTOR_MAX_PWM) right_target = MOTOR_MAX_PWM;
  if (right_target < -MOTOR_MAX_PWM) right_target = -MOTOR_MAX_PWM;

  g_target_left = left_target;
  g_target_right = right_target;
}

bool pwm_controller_tick() {
  unsigned long now = millis();
  if (now - g_last_ramp_ms < PWM_RAMP_INTERVAL) {
    return false;
  }
  g_last_ramp_ms = now;

  int prev_left = g_current_left;
  int prev_right = g_current_right;

  g_current_left = ramp_towards(g_current_left, g_target_left, PWM_RAMP_STEP);
  g_current_right = ramp_towards(g_current_right, g_target_right, PWM_RAMP_STEP);

  return (g_current_left != prev_left || g_current_right != prev_right);
}

int pwm_controller_get_left() {
  return g_current_left;
}

int pwm_controller_get_right() {
  return g_current_right;
}
