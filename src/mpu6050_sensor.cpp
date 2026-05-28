#include "mpu6050_sensor.h"
#include "config.h"
#include <Wire.h>

namespace {
  SensorTelemetry g_telemetry;
  uint32_t g_last_read_ms = 0;

  // I2C Registers for MPU6050
  constexpr uint8_t MPU_PWR_MGMT_1 = 0x6B;
  constexpr uint8_t MPU_ACCEL_XOUT_H = 0x3B;

  void determine_tilt(float x, float y) {
    float ax = abs(x);
    float ay = abs(y);
    
    if (ax < 0.2f && ay < 0.2f) {
      strncpy(g_telemetry.tilt, "FLAT", sizeof(g_telemetry.tilt));
    } else if (ax > ay) {
      if (x > 0.4f) {
        strncpy(g_telemetry.tilt, "RIGHT", sizeof(g_telemetry.tilt));
      } else if (x < -0.4f) {
        strncpy(g_telemetry.tilt, "LEFT", sizeof(g_telemetry.tilt));
      }
    } else {
      if (y > 0.4f) {
        strncpy(g_telemetry.tilt, "DOWN", sizeof(g_telemetry.tilt));
      } else if (y < -0.4f) {
        strncpy(g_telemetry.tilt, "UP", sizeof(g_telemetry.tilt));
      }
    }
  }
}

bool mpu6050_init() {
  Serial.println("[MPU] Initializing MPU6050...");
  
  // Initialize I2C bus
  Wire.begin(config::kMpuSdaPin, config::kMpuSclPin);
  Wire.setClock(400000); // 400kHz Fast Mode

  // Wake up MPU6050
  Wire.beginTransmission(config::kMpuAddr);
  Wire.write(MPU_PWR_MGMT_1);
  Wire.write(0x00); // Clear sleep bit
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("[MPU] MPU6050 connection successful.");
    g_telemetry.connected = true;
    return true;
  } else {
    Serial.print("[MPU ERROR] MPU6050 connection failed! Error: ");
    Serial.println(error);
    g_telemetry.connected = false;
    return false;
  }
}

void mpu6050_tick() {
  if (!g_telemetry.connected) {
    return;
  }

  uint32_t now = millis();
  if (now - g_last_read_ms >= config::kMpuReadIntervalMs) {
    g_last_read_ms = now;

    Wire.beginTransmission(config::kMpuAddr);
    Wire.write(MPU_ACCEL_XOUT_H);
    if (Wire.endTransmission(false) != 0) {
       g_telemetry.connected = false; // Lost connection
       return;
    }

    if (Wire.requestFrom((uint16_t)config::kMpuAddr, (uint8_t)6, (uint8_t)true) == 6) {
      int16_t ax = (Wire.read() << 8) | Wire.read();
      int16_t ay = (Wire.read() << 8) | Wire.read();
      int16_t az = (Wire.read() << 8) | Wire.read();

      // Convert to g (using default +/- 2g range: 16384 LSB/g)
      g_telemetry.accel_x = ax / 16384.0f;
      g_telemetry.accel_y = ay / 16384.0f;
      g_telemetry.accel_z = az / 16384.0f;
      g_telemetry.accel_total = sqrt(g_telemetry.accel_x * g_telemetry.accel_x + 
                                     g_telemetry.accel_y * g_telemetry.accel_y + 
                                     g_telemetry.accel_z * g_telemetry.accel_z);

      determine_tilt(g_telemetry.accel_x, g_telemetry.accel_y);
      g_telemetry.last_update_ms = now;
    } else {
      g_telemetry.connected = false;
    }
  }
}

const SensorTelemetry *mpu6050_get_telemetry() {
  return &g_telemetry;
}
