#pragma once

#include <Arduino.h>

// Initialize the SoftwareSerial bridge to the ESP32-CAM.
void serial_protocol_init();

// Call from loop(). Reads incoming bytes and assembles complete lines.
// When a full line is received, it is passed to the movement parser.
void serial_protocol_tick();

// Send a string to the ESP32-CAM.
void serial_protocol_send(const char *msg);

// Send a formatted telemetry line to the ESP32-CAM.
void serial_protocol_send_telemetry(float x, float y, float z, float accel, const char *tilt);

// Send the current movement state to the ESP32-CAM.
void serial_protocol_send_state(const char *state);

// Send a STATUS:OK heartbeat.
void serial_protocol_send_status();
