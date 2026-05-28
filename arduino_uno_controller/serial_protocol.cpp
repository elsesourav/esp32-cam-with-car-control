#include "serial_protocol.h"
#include "movement_parser.h"
#include "config.h"
#include <SoftwareSerial.h>
#include <stdio.h>

namespace {
SoftwareSerial g_bridge(BRIDGE_RX_PIN, BRIDGE_TX_PIN);

char g_line_buf[64];
int g_line_pos = 0;

void process_line(const char *line) {
  Serial.print(F("[RX] "));
  Serial.println(line);

  if (!movement_parser_handle(line)) {
    Serial.print(F("[RX] Unknown command: "));
    Serial.println(line);
  }
}
}  // namespace

void serial_protocol_init() {
  g_bridge.begin(BRIDGE_BAUD);
  Serial.println(F("[Serial] Bridge initialized"));
}

void serial_protocol_tick() {
  while (g_bridge.available()) {
    char c = g_bridge.read();
    if (c == '\n' || c == '\r') {
      if (g_line_pos > 0) {
        g_line_buf[g_line_pos] = '\0';
        process_line(g_line_buf);
        g_line_pos = 0;
      }
    } else {
      if (g_line_pos < (int)sizeof(g_line_buf) - 1) {
        g_line_buf[g_line_pos++] = c;
      }
    }
  }
}

void serial_protocol_send(const char *msg) {
  g_bridge.println(msg);
}

void serial_protocol_send_telemetry(float x, float y, float z, float accel, const char *tilt) {
  char buf[80];
  // dtostrf for float formatting on AVR
  char sx[8], sy[8], sz[8], sa[8];
  dtostrf(x, 1, 2, sx);
  dtostrf(y, 1, 2, sy);
  dtostrf(z, 1, 2, sz);
  dtostrf(accel, 1, 2, sa);

  snprintf(buf, sizeof(buf), "MPU:X=%s,Y=%s,Z=%s,A=%s,T=%s", sx, sy, sz, sa, tilt);
  g_bridge.println(buf);
}

void serial_protocol_send_state(const char *state) {
  g_bridge.print(F("STATE:"));
  g_bridge.println(state);
}

void serial_protocol_send_status() {
  g_bridge.println(F("STATUS:OK"));
}
