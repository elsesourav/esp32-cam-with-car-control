#pragma once

// =============================================================
// Arduino Uno Motor Controller Configuration
// =============================================================

// --- UART Communication (SoftwareSerial) ---
// Connected to ESP32-CAM UART2 (GPIO 14 TX, GPIO 15 RX)
#define BRIDGE_RX_PIN   10   // Arduino pin 10 receives from ESP32 TX
#define BRIDGE_TX_PIN   11   // Arduino pin 11 transmits to ESP32 RX
#define BRIDGE_BAUD     38400

// --- L298N Motor Driver #1 (Left Side) ---
// Controls Front-Left and Rear-Left motors (wired in parallel)
#define LEFT_IN1        2    // Digital pin → IN1 (forward)
#define LEFT_IN2        4    // Digital pin → IN2 (backward)
#define LEFT_ENA        3    // PWM pin → ENA (speed control)

// --- L298N Motor Driver #2 (Right Side) ---
// Controls Front-Right and Rear-Right motors (wired in parallel)
#define RIGHT_IN3       7    // Digital pin → IN3 (forward)
#define RIGHT_IN4       8    // Digital pin → IN4 (backward)
#define RIGHT_ENB       5    // PWM pin → ENB (speed control)

// --- Motor Limits ---
#define MOTOR_MAX_PWM       255
#define MOTOR_MIN_PWM       0
#define PWM_RAMP_STEP       15   // PWM units per ramp tick
#define PWM_RAMP_INTERVAL   20   // ms between ramp steps

// --- Command Timeout ---
// If no movement command is received within this time, auto-stop.
#define COMMAND_TIMEOUT_MS  500

// --- MPU6050 ---
#define MPU6050_ADDR        0x68
#define TELEMETRY_INTERVAL  200  // ms between telemetry sends
#define STATUS_INTERVAL     2000 // ms between STATUS:OK heartbeats
