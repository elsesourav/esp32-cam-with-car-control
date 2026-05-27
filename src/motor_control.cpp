#include "motor_control.h"

#include "config.h"
#include "esp32-hal-ledc.h"

namespace {
struct MotorPairPins {
  int in1;
  int in2;
  int ena;
  int in3;
  int in4;
  int enb;
};

const MotorPairPins kLeftPins{
  config::kLeftIn1,
  config::kLeftIn2,
  config::kLeftEna,
  config::kLeftIn3,
  config::kLeftIn4,
  config::kLeftEnb,
};

const MotorPairPins kRightPins{
  config::kRightIn1,
  config::kRightIn2,
  config::kRightEna,
  config::kRightIn3,
  config::kRightIn4,
  config::kRightEnb,
};

uint32_t g_last_command_ms = 0;

void setup_motor_pair(const MotorPairPins &pins) {
  pinMode(pins.in1, OUTPUT);
  pinMode(pins.in2, OUTPUT);
  pinMode(pins.in3, OUTPUT);
  pinMode(pins.in4, OUTPUT);

  ledcAttach(pins.ena, config::kMotorPwmFreq, config::kMotorPwmResolution);
  ledcAttach(pins.enb, config::kMotorPwmFreq, config::kMotorPwmResolution);
}

void set_motor_output(const MotorPairPins &pins, int speed) {
  int duty = abs(speed);
  if (duty > config::kMotorMaxPwm) {
    duty = config::kMotorMaxPwm;
  }

  bool forward = speed > 0;
  bool backward = speed < 0;

  digitalWrite(pins.in1, forward ? HIGH : LOW);
  digitalWrite(pins.in2, backward ? HIGH : LOW);
  digitalWrite(pins.in3, forward ? HIGH : LOW);
  digitalWrite(pins.in4, backward ? HIGH : LOW);

  if (speed == 0) {
    ledcWrite(pins.ena, 0);
    ledcWrite(pins.enb, 0);
  } else {
    ledcWrite(pins.ena, duty);
    ledcWrite(pins.enb, duty);
  }
}

int clamp_pwm(int value) {
  if (value > config::kMotorMaxPwm) {
    return config::kMotorMaxPwm;
  }
  if (value < -config::kMotorMaxPwm) {
    return -config::kMotorMaxPwm;
  }
  return value;
}
}  // namespace

void motor_control_init() {
  setup_motor_pair(kLeftPins);
  setup_motor_pair(kRightPins);
  motor_control_stop();
  g_last_command_ms = millis();
}

void motor_control_tick(uint32_t now_ms) {
  if (now_ms - g_last_command_ms > config::kCommandTimeoutMs) {
    motor_control_stop();
  }
}

void motor_control_mark_command(uint32_t now_ms) {
  g_last_command_ms = now_ms;
}

void motor_control_set_differential(int left_speed, int right_speed) {
  static int s_last_left = -999;
  static int s_last_right = -999;
  int left = clamp_pwm(left_speed);
  int right = clamp_pwm(right_speed);
  
  if (left != s_last_left || right != s_last_right) {
    Serial.print("Motor Action: L = ");
    Serial.print(left);
    Serial.print(" | R = ");
    Serial.println(right);
    s_last_left = left;
    s_last_right = right;
  }

  set_motor_output(kLeftPins, left);
  set_motor_output(kRightPins, right);
}

void motor_control_stop() {
  set_motor_output(kLeftPins, 0);
  set_motor_output(kRightPins, 0);
}

void motor_control_forward(uint8_t speed) {
  motor_control_set_differential(speed, speed);
}

void motor_control_backward(uint8_t speed) {
  motor_control_set_differential(-speed, -speed);
}

void motor_control_left(uint8_t speed) {
  motor_control_set_differential(-speed, speed);
}

void motor_control_right(uint8_t speed) {
  motor_control_set_differential(speed, -speed);
}

void motor_control_forward_left(uint8_t speed, uint8_t turn) {
  motor_control_set_differential(speed - turn, speed + turn);
}

void motor_control_forward_right(uint8_t speed, uint8_t turn) {
  motor_control_set_differential(speed + turn, speed - turn);
}

void motor_control_backward_left(uint8_t speed, uint8_t turn) {
  motor_control_set_differential(-(speed - turn), -(speed + turn));
}

void motor_control_backward_right(uint8_t speed, uint8_t turn) {
  motor_control_set_differential(-(speed + turn), -(speed - turn));
}
