#include "motor_control.h"

#include "config.h"
#include "esp32-hal-ledc.h"

namespace {
struct MotorPins {
  int direction;
  int pwm;
};

const MotorPins kMotorAPins{config::kMotorAFwd, config::kMotorAPwm};
const MotorPins kMotorBPins{config::kMotorBFwd, config::kMotorBPwm};
const MotorPins kMotorCPins{config::kMotorCFwd, config::kMotorCPwm};
const MotorPins kMotorDPins{config::kMotorDFwd, config::kMotorDPwm};

uint32_t g_last_command_ms = 0;

void setup_motor(const MotorPins &pins, bool safe_high = false) {
  pinMode(pins.direction, OUTPUT);
  pinMode(pins.pwm, OUTPUT);

  // Put boot-sensitive pins in a safe default before attaching PWM.
  if (safe_high) {
    digitalWrite(pins.direction, HIGH);
  } else {
    digitalWrite(pins.direction, LOW);
  }

  ledcAttach(pins.pwm, config::kMotorPwmFreq, config::kMotorPwmResolution);
}

void set_one_motor(const MotorPins &pins, int speed) {
  int duty = abs(speed);
  if (duty > config::kMotorMaxPwm) {
    duty = config::kMotorMaxPwm;
  }

  if (speed == 0) {
    ledcWrite(pins.pwm, 0);
    // Keep output direction stable while stopped.
    return;
  }

  const bool forward = speed > 0;
  digitalWrite(pins.direction, forward ? HIGH : LOW);
  ledcWrite(pins.pwm, duty);
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
  setup_motor(kMotorAPins, false);
  setup_motor(kMotorBPins, false);
  setup_motor(kMotorCPins, false);
  setup_motor(kMotorDPins, false);

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

  // A and C are left side, B and D are right side.
  set_one_motor(kMotorAPins, left);
  set_one_motor(kMotorCPins, left);
  set_one_motor(kMotorBPins, right);
  set_one_motor(kMotorDPins, right);
}

void motor_control_stop() {
  set_one_motor(kMotorAPins, 0);
  set_one_motor(kMotorBPins, 0);
  set_one_motor(kMotorCPins, 0);
  set_one_motor(kMotorDPins, 0);
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
