#pragma once

#include <Arduino.h>

// Initialize the motor controller pins and set all motors to safe stop.
void motor_controller_init();

// Set differential drive: left_pwm and right_pwm range from -255 to +255.
// Positive = forward, negative = backward, 0 = stop.
void motor_controller_set_differential(int left_pwm, int right_pwm);

// Convenience: stop all motors immediately.
void motor_controller_stop();
