#pragma once

#include <Arduino.h>

enum class MovementState {
  kStop,
  kForward,
  kBackward,
  kLeft,
  kRight,
  kForwardLeft,
  kForwardRight,
  kBackwardLeft,
  kBackwardRight,
};

void motor_control_init();
void motor_control_tick(uint32_t now_ms);

void motor_control_stop();
void motor_control_forward(uint8_t speed);
void motor_control_backward(uint8_t speed);
void motor_control_left(uint8_t speed);
void motor_control_right(uint8_t speed);
void motor_control_forward_left(uint8_t speed, uint8_t turn);
void motor_control_forward_right(uint8_t speed, uint8_t turn);
void motor_control_backward_left(uint8_t speed, uint8_t turn);
void motor_control_backward_right(uint8_t speed, uint8_t turn);

void motor_control_set_differential(int left_speed, int right_speed);
void motor_control_mark_command(uint32_t now_ms);
