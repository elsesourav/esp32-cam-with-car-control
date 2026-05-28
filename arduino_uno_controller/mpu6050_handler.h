#pragma once

#include <Arduino.h>

// Initialize the MPU6050 sensor over I2C.
// Returns true if the sensor was found and configured.
bool mpu6050_init();

// Call from loop(). Reads the latest sensor values.
void mpu6050_tick();

// Get the latest filtered accelerometer values (in g).
float mpu6050_get_x();
float mpu6050_get_y();
float mpu6050_get_z();

// Get total acceleration magnitude.
float mpu6050_get_total_accel();

// Get a human-readable tilt direction string.
// Returns: "FLAT", "TILT_FWD", "TILT_BACK", "TILT_LEFT", "TILT_RIGHT"
const char *mpu6050_get_tilt();

// Returns true if the sensor is connected and responding.
bool mpu6050_is_connected();
