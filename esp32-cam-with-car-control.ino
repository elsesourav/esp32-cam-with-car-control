#include <Arduino.h>
#include <WiFi.h>

#include "src/camera_control.h"
#include "src/config.h"
#include "src/flashlight_control.h"
#include "src/serial_bridge.h"
#include "src/websocket_handler.h"

namespace {
void connect_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(config::kWifiSsid, config::kWifiPass);

  uint32_t start_ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(150);
    Serial.print(".");
    if (millis() - start_ms > 15000) {
      start_ms = millis();
    }
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}
}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  connect_wifi();
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());

  // Initialize UART bridge to Arduino Uno (replaces motor_control_init).
  serial_bridge_init();

  flashlight_init();

  if (!camera_control_init()) {
    Serial.println("Camera init failed");
  }

  camera_control_set_stream_enabled(false);
  camera_control_start_stream_server(config::kStreamPort);

  Serial.println("Starting HTTP server...");
  if (!websocket_handler_start_server()) {
    Serial.println("HTTP server start failed");
  } else {
    Serial.println("HTTP server ready");
  }

  Serial.print("Control UI: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.print("Stream URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream");
}

void loop() {
  serial_bridge_tick();
  websocket_handler_tick(millis());
  delay(2);
}
