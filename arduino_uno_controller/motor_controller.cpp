#include "motor_controller.h"
#include "config.h"

namespace {
// Set a single L298N channel.
// in1/in2 control direction, en controls PWM speed.
void set_channel(uint8_t in1, uint8_t in2, uint8_t en, int speed) {
  int duty = abs(speed);
  if (duty > MOTOR_MAX_PWM) duty = MOTOR_MAX_PWM;

  if (speed > 0) {
    // Forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    // Backward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    // Stop (brake)
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  analogWrite(en, duty);
}
}  // namespace

void motor_controller_init() {
  // Left channel
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(LEFT_ENA, OUTPUT);

  // Right channel
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  pinMode(RIGHT_ENB, OUTPUT);

  // Start in safe stop state.
  motor_controller_stop();

  Serial.println(F("[Motor] Initialized"));
}

void motor_controller_set_differential(int left_pwm, int right_pwm) {
  set_channel(LEFT_IN1, LEFT_IN2, LEFT_ENA, left_pwm);
  set_channel(RIGHT_IN3, RIGHT_IN4, RIGHT_ENB, right_pwm);
}

void motor_controller_stop() {
  set_channel(LEFT_IN1, LEFT_IN2, LEFT_ENA, 0);
  set_channel(RIGHT_IN3, RIGHT_IN4, RIGHT_ENB, 0);
}
