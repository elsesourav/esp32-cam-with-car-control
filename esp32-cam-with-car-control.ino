#include <Arduino.h>
#include <WiFi.h>

#include "src/camera_control.h"
#include "src/config.h"
#include "src/flashlight_control.h"
#include "src/motor_control.h"
#include "src/mpu6050_sensor.h"
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
  // Pass -1 for the RX pin so that GPIO 3 is NOT used by the Serial Monitor.
  // This frees up GPIO 3 to be used as the Right Motor Backward PWM output!
  Serial.begin(115200, SERIAL_8N1, -1, 1);
  Serial.setDebugOutput(true);

  connect_wifi();
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());

  Serial.println("[BOOT] Initializing standalone ESP32-CAM...");
  motor_control_init();

  flashlight_init();

  if (!camera_control_init()) {
    Serial.println("[BOOT ERROR] Camera init failed");
  } 

  // MUST initialize I2C after camera init to avoid bus conflict
  mpu6050_init();

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
  motor_control_tick();
  mpu6050_tick();
  websocket_handler_tick(millis());
  delay(1);  // Reduced from 2ms for faster motor/control responsiveness
}
