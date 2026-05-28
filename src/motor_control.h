#pragma once

#include <Arduino.h>

// Initialize the L298N motor driver pins and PWM channels.
void motor_control_init();

// Set motor speeds. Values range from -255 (full reverse) to +255 (full forward).
// The PWM ramp system will smoothly transition to these targets.
void motor_control_set(int left_speed, int right_speed);

// Emergency stop — immediately sets all motor pins LOW (no ramp).
void motor_control_stop();

// Call from loop(). Handles PWM ramping toward the target speeds.
void motor_control_tick();

// Returns a human-readable state string for telemetry (e.g., "FORWARD", "LEFT", "STOP").
const char *motor_control_get_state();
