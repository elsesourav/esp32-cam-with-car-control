#pragma once

#include <Arduino.h>

// Telemetry data from the MPU6050 sensor.
struct SensorTelemetry {
  float accel_x = 0;
  float accel_y = 0;
  float accel_z = 0;
  float accel_total = 0;
  char tilt[16] = "UNKNOWN";
  bool connected = false;
  uint32_t last_update_ms = 0;
};

// Initialize the MPU6050 sensor over I2C.
// Call AFTER camera_control_init() since they share the I2C bus.
bool mpu6050_init();

// Call from loop(). Performs non-blocking periodic reads.
void mpu6050_tick();

// Returns a pointer to the latest telemetry snapshot.
const SensorTelemetry *mpu6050_get_telemetry();
