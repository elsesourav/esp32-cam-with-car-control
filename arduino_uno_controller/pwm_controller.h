#pragma once

#include <Arduino.h>

// Initialize PWM smoothing state.
void pwm_controller_init();

// Set the target PWM values. The actual output will ramp towards these.
void pwm_controller_set_target(int left_target, int right_target);

// Call from loop(). Smoothly ramps current PWM towards targets.
// Returns true if the values changed (so caller can update motors).
bool pwm_controller_tick();

// Get current smoothed PWM values.
int pwm_controller_get_left();
int pwm_controller_get_right();
