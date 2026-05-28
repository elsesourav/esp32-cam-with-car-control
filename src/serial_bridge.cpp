#include "serial_bridge.h"

#include "config.h"
#include <string.h>

namespace {
// UART2 for communication with Arduino Uno.
HardwareSerial &g_bridge = Serial2;

BridgeTelemetry g_telemetry;

// Line buffer for incoming serial data.
char g_line_buf[128];
int g_line_pos = 0;

// Timeout: if no data received for this long, mark disconnected.
constexpr uint32_t kDisconnectTimeoutMs = 3000;

// Parse a telemetry line from the Arduino.
// Supported formats:
//   MPU:X=0.12,Y=-0.45,Z=9.80,A=1.23,T=FLAT
//   STATE:FORWARD
//   STATUS:OK
void parse_line(const char *line) {
  if (strncmp(line, "MPU:", 4) == 0) {
    const char *p = line + 4;

    // Parse X=value
    const char *xp = strstr(p, "X=");
    const char *yp = strstr(p, "Y=");
    const char *zp = strstr(p, "Z=");
    const char *ap = strstr(p, "A=");
    const char *tp = strstr(p, "T=");

    if (xp) g_telemetry.accel_x = atof(xp + 2);
    if (yp) g_telemetry.accel_y = atof(yp + 2);
    if (zp) g_telemetry.accel_z = atof(zp + 2);
    if (ap) g_telemetry.accel_total = atof(ap + 2);
    if (tp) {
      // Copy tilt string until comma or end of line
      const char *src = tp + 2;
      int i = 0;
      while (*src && *src != ',' && *src != '\r' && *src != '\n' && i < 15) {
        g_telemetry.tilt[i++] = *src++;
      }
      g_telemetry.tilt[i] = '\0';
    }

    g_telemetry.connected = true;
    g_telemetry.last_update_ms = millis();

  } else if (strncmp(line, "STATE:", 6) == 0) {
    const char *src = line + 6;
    int i = 0;
    while (*src && *src != '\r' && *src != '\n' && i < 15) {
      g_telemetry.state[i++] = *src++;
    }
    g_telemetry.state[i] = '\0';

    g_telemetry.connected = true;
    g_telemetry.last_update_ms = millis();

  } else if (strncmp(line, "STATUS:", 7) == 0) {
    g_telemetry.connected = true;
    g_telemetry.last_update_ms = millis();
  }
}
}  // namespace

void serial_bridge_init() {
  g_bridge.begin(config::kBridgeBaud, SERIAL_8N1,
                 config::kBridgeRxPin, config::kBridgeTxPin);

  Serial.println("[Bridge] UART2 initialized");
  Serial.print("[Bridge] TX=GPIO");
  Serial.print(config::kBridgeTxPin);
  Serial.print(" RX=GPIO");
  Serial.print(config::kBridgeRxPin);
  Serial.print(" Baud=");
  Serial.println(config::kBridgeBaud);
}

void serial_bridge_send(const char *cmd) {
  g_bridge.print(cmd);
  g_bridge.print('\n');

  Serial.print("[Bridge TX] ");
  Serial.println(cmd);
}

void serial_bridge_tick() {
  // Read all available bytes and assemble lines.
  while (g_bridge.available()) {
    char c = g_bridge.read();
    if (c == '\n' || c == '\r') {
      if (g_line_pos > 0) {
        g_line_buf[g_line_pos] = '\0';
        Serial.print("[Bridge RX] ");
        Serial.println(g_line_buf);
        parse_line(g_line_buf);
        g_line_pos = 0;
      }
    } else {
      if (g_line_pos < (int)sizeof(g_line_buf) - 1) {
        g_line_buf[g_line_pos++] = c;
      }
    }
  }

  // Check for disconnect timeout.
  if (g_telemetry.connected &&
      millis() - g_telemetry.last_update_ms > kDisconnectTimeoutMs) {
    g_telemetry.connected = false;
    Serial.println("[Bridge] Arduino disconnected (timeout)");
  }
}

const BridgeTelemetry *serial_bridge_get_telemetry() {
  return &g_telemetry;
}
