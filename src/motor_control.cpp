#include "motor_control.h"
#include "config.h"
#include "esp32-hal-ledc.h"

// Note: ESP32 Core v3+ uses pins directly for ledcWrite, so we don't need channel definitions.

namespace {
  int g_target_left = 0;
  int g_target_right = 0;
  int g_current_left = 0;
  int g_current_right = 0;
  uint32_t g_last_tick_ms = 0;
  char g_state_str[16] = "STOP";

  int clamp_pwm(int val) {
    if (val < -config::kMotorMaxPwm) return -config::kMotorMaxPwm;
    if (val > config::kMotorMaxPwm) return config::kMotorMaxPwm;
    return val;
  }

  void update_motor_hardware() {
    // Left Motor
    if (g_current_left > 0) {
      ledcWrite(config::kMotorLeftIn1, g_current_left);
      ledcWrite(config::kMotorLeftIn2, 0);
    } else if (g_current_left < 0) {
      ledcWrite(config::kMotorLeftIn1, 0);
      ledcWrite(config::kMotorLeftIn2, -g_current_left);
    } else {
      ledcWrite(config::kMotorLeftIn1, 0);
      ledcWrite(config::kMotorLeftIn2, 0);
    }

    // Right Motor
    if (g_current_right > 0) {
      ledcWrite(config::kMotorRightIn3, g_current_right);
      ledcWrite(config::kMotorRightIn4, 0);
    } else if (g_current_right < 0) {
      ledcWrite(config::kMotorRightIn3, 0);
      ledcWrite(config::kMotorRightIn4, -g_current_right);
    } else {
      ledcWrite(config::kMotorRightIn3, 0);
      ledcWrite(config::kMotorRightIn4, 0);
    }
    
    // Update string state
    if (g_target_left == 0 && g_target_right == 0) {
      snprintf(g_state_str, sizeof(g_state_str), "STOP");
    } else if (g_target_left > 0 && g_target_right > 0) {
      snprintf(g_state_str, sizeof(g_state_str), "FORWARD");
    } else if (g_target_left < 0 && g_target_right < 0) {
      snprintf(g_state_str, sizeof(g_state_str), "BACKWARD");
    } else if (g_target_left < 0 && g_target_right > 0) {
      snprintf(g_state_str, sizeof(g_state_str), "LEFT");
    } else if (g_target_left > 0 && g_target_right < 0) {
      snprintf(g_state_str, sizeof(g_state_str), "RIGHT");
    } else {
      snprintf(g_state_str, sizeof(g_state_str), "MOVING");
    }
  }

  int step_toward(int current, int target, int step) {
    if (current < target) {
      current += step;
      if (current > target) current = target;
    } else if (current > target) {
      current -= step;
      if (current < target) current = target;
    }
    return current;
  }
}

void motor_control_init() {
  Serial.println("[MOTOR] Initializing L298N 4-Pin PWM driver...");

  // Attach pins to LEDC with frequency and resolution (ESP32 Core v3+ API)
  ledcAttach(config::kMotorLeftIn1, config::kMotorPwmFreq, config::kMotorPwmResolution);
  ledcAttach(config::kMotorLeftIn2, config::kMotorPwmFreq, config::kMotorPwmResolution);
  ledcAttach(config::kMotorRightIn3, config::kMotorPwmFreq, config::kMotorPwmResolution);
  ledcAttach(config::kMotorRightIn4, config::kMotorPwmFreq, config::kMotorPwmResolution);

  motor_control_stop();
  Serial.println("[MOTOR] Init complete.");
}

void motor_control_set(int left_speed, int right_speed) {
  g_target_left = clamp_pwm(left_speed);
  g_target_right = clamp_pwm(right_speed);
}

void motor_control_stop() {
  g_target_left = 0;
  g_target_right = 0;
  g_current_left = 0;
  g_current_right = 0;
  update_motor_hardware();
}

void motor_control_tick() {
  uint32_t now = millis();
  if (now - g_last_tick_ms >= config::kMotorRampIntervalMs) {
    g_last_tick_ms = now;

    if (g_current_left != g_target_left || g_current_right != g_target_right) {
      g_current_left = step_toward(g_current_left, g_target_left, config::kMotorRampStep);
      g_current_right = step_toward(g_current_right, g_target_right, config::kMotorRampStep);
      update_motor_hardware();
    }
  }
}

const char *motor_control_get_state() {
  return g_state_str;
}
