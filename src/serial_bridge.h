#pragma once

#include <Arduino.h>

// Telemetry data received from the Arduino Uno via UART.
struct BridgeTelemetry {
  float accel_x = 0;
  float accel_y = 0;
  float accel_z = 0;
  float accel_total = 0;
  char tilt[16] = "UNKNOWN";
  char state[16] = "STOP";
  bool connected = false;
  uint32_t last_update_ms = 0;
};

// Initialize the UART bridge (Serial2) to the Arduino Uno.
void serial_bridge_init();

// Send a command string to the Arduino Uno. Appends newline automatically.
// Example: serial_bridge_send("MOVE:F") sends "MOVE:F\n"
void serial_bridge_send(const char *cmd);

// Call from loop(). Reads and parses incoming telemetry lines from the Arduino.
void serial_bridge_tick();

// Returns a pointer to the latest telemetry snapshot.
const BridgeTelemetry *serial_bridge_get_telemetry();
