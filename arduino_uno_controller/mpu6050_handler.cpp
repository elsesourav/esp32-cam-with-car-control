#include "mpu6050_handler.h"
#include "config.h"
#include <Wire.h>
#include <math.h>

namespace {
bool g_connected = false;

// Raw accelerometer values converted to g.
float g_ax = 0, g_ay = 0, g_az = 0;

// Simple low-pass filter coefficient (0..1, lower = smoother).
constexpr float kAlpha = 0.3f;

// Filtered values.
float g_fx = 0, g_fy = 0, g_fz = 0;

// Tilt threshold in g.
constexpr float kTiltThreshold = 0.3f;

void read_raw(int16_t *ax, int16_t *ay, int16_t *az) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H register
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)6, (uint8_t)true);

  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
}
}  // namespace

bool mpu6050_init() {
  Wire.begin();

  // Wake up MPU6050 (clear sleep bit).
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0x00);  // Wake up
  uint8_t err = Wire.endTransmission();

  if (err != 0) {
    Serial.println(F("[MPU6050] Not found!"));
    g_connected = false;
    return false;
  }

  // Configure accelerometer to ±2g range.
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x1C);  // ACCEL_CONFIG register
  Wire.write(0x00);  // ±2g
  Wire.endTransmission();

  // Configure low-pass filter for smoother readings.
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x1A);  // CONFIG register
  Wire.write(0x03);  // DLPF ~44Hz bandwidth
  Wire.endTransmission();

  g_connected = true;
  Serial.println(F("[MPU6050] Initialized"));
  return true;
}

void mpu6050_tick() {
  if (!g_connected) return;

  int16_t raw_ax, raw_ay, raw_az;
  read_raw(&raw_ax, &raw_ay, &raw_az);

  // Convert to g (±2g range → 16384 LSB/g).
  g_ax = raw_ax / 16384.0f;
  g_ay = raw_ay / 16384.0f;
  g_az = raw_az / 16384.0f;

  // Apply simple low-pass filter.
  g_fx = g_fx + kAlpha * (g_ax - g_fx);
  g_fy = g_fy + kAlpha * (g_ay - g_fy);
  g_fz = g_fz + kAlpha * (g_az - g_fz);
}

float mpu6050_get_x() { return g_fx; }
float mpu6050_get_y() { return g_fy; }
float mpu6050_get_z() { return g_fz; }

float mpu6050_get_total_accel() {
  return sqrt(g_fx * g_fx + g_fy * g_fy + g_fz * g_fz);
}

const char *mpu6050_get_tilt() {
  float ax = fabs(g_fx);
  float ay = fabs(g_fy);

  if (ax < kTiltThreshold && ay < kTiltThreshold) {
    return "FLAT";
  }

  // Find dominant axis.
  if (ax > ay) {
    return (g_fx > 0) ? "TILT_RIGHT" : "TILT_LEFT";
  } else {
    return (g_fy > 0) ? "TILT_FWD" : "TILT_BACK";
  }
}

bool mpu6050_is_connected() {
  return g_connected;
}
