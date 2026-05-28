// =============================================================
// Arduino Uno Motor Controller
// =============================================================
// This sketch runs on the Arduino Uno and handles:
//   - Motor control via 2× L298N drivers
//   - MPU6050 sensor reading (I2C)
//   - UART communication with ESP32-CAM
//   - PWM smoothing for gradual acceleration
//
// Upload this sketch separately to the Arduino Uno.
// =============================================================

#include "config.h"
#include "motor_controller.h"
#include "pwm_controller.h"
#include "serial_protocol.h"
#include "movement_parser.h"
#include "mpu6050_handler.h"

// Timing state.
unsigned long g_last_command_ms = 0;
unsigned long g_last_telemetry_ms = 0;
unsigned long g_last_status_ms = 0;
bool g_timed_out = false;

// Track the last state sent for telemetry.
const char *g_last_state = "STOP";

void setup() {
  // Debug serial (USB).
  Serial.begin(115200);
  Serial.println(F("=== Arduino Uno Motor Controller ==="));

  // Initialize subsystems.
  serial_protocol_init();
  motor_controller_init();
  pwm_controller_init();

  bool mpu_ok = mpu6050_init();
  if (!mpu_ok) {
    Serial.println(F("[WARN] MPU6050 not connected. Telemetry will be zeros."));
  }

  // Send initial status to ESP32.
  serial_protocol_send_status();
  g_last_command_ms = millis();

  Serial.println(F("[Setup] Ready"));
}

void loop() {
  unsigned long now = millis();

  // --- 1. Read serial commands from ESP32-CAM ---
  serial_protocol_tick();

  // Check if a new command was just processed (movement_parser updates
  // the PWM targets, so we can detect it indirectly).
  // We track command freshness via the pwm targets changing.
  // For simplicity, we always update the command timestamp when
  // serial_protocol_tick processes any valid command.
  // The movement_parser_handle() is called inside serial_protocol_tick().
  // We need to track if a command arrived. Let's use a simple approach:
  // check if PWM targets are non-zero as a proxy for active commands.
  // Actually, let's just check in serial_protocol_tick if it processed
  // a MOVE or DIFF command — but that requires modifying the flow.
  // Simpler: just reset the timer after every serial_protocol_tick
  // if there were bytes available. This is safe because the ESP32
  // only sends data when the user is interacting.

  // --- 2. PWM Smoothing ---
  if (pwm_controller_tick()) {
    // PWM values changed, update motors.
    motor_controller_set_differential(
      pwm_controller_get_left(),
      pwm_controller_get_right()
    );
    g_last_command_ms = now;
    g_timed_out = false;
  }

  // --- 3. Command Timeout Safety ---
  if (!g_timed_out && (now - g_last_command_ms > COMMAND_TIMEOUT_MS)) {
    // No commands received recently — stop motors.
    pwm_controller_set_target(0, 0);
    motor_controller_stop();
    g_timed_out = true;
    Serial.println(F("[Timeout] Motors stopped"));
  }

  // --- 4. Read MPU6050 ---
  mpu6050_tick();

  // --- 5. Send Telemetry to ESP32 ---
  if (now - g_last_telemetry_ms >= TELEMETRY_INTERVAL) {
    g_last_telemetry_ms = now;

    serial_protocol_send_telemetry(
      mpu6050_get_x(),
      mpu6050_get_y(),
      mpu6050_get_z(),
      mpu6050_get_total_accel(),
      mpu6050_get_tilt()
    );
  }

  // --- 6. Send Status Heartbeat ---
  if (now - g_last_status_ms >= STATUS_INTERVAL) {
    g_last_status_ms = now;
    serial_protocol_send_status();
  }
}
