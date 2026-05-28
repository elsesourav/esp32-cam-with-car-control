#include "websocket_handler.h"

#include <Arduino.h>

#include "camera_control.h"
#include "config.h"
#include "flashlight_control.h"
#include "serial_bridge.h"
#include "movement_parser.h"

#include "esp_http_server.h"
#include "embedded_files.h"
#include <string.h>

namespace {
httpd_handle_t g_httpd = NULL;
MovementSettings g_settings;

// Telemetry broadcast state.
uint32_t g_last_telemetry_broadcast_ms = 0;

// Cached telemetry string for comparing changes.
char g_last_telemetry_str[128] = {0};

const char *get_content_type(const char *path) {
  if (strstr(path, ".html")) {
    return "text/html";
  }
  if (strstr(path, ".css")) {
    return "text/css";
  }
  if (strstr(path, ".js")) {
    return "application/javascript";
  }
  if (strstr(path, ".png")) {
    return "image/png";
  }
  if (strstr(path, ".svg")) {
    return "image/svg+xml";
  }
  return "text/plain";
}

esp_err_t file_handler(httpd_req_t *req) {
  char path[64] = {0};
  if (strcmp(req->uri, "/") == 0) {
    snprintf(path, sizeof(path), "/index.html");
  } else {
    snprintf(path, sizeof(path), "%s", req->uri);
  }

  Serial.print("HTTP GET ");
  Serial.print(req->uri);
  Serial.print(" -> ");
  Serial.println(path);

  const char* out_data = nullptr;
  size_t out_len = 0;

  if (strcmp(path, "/index.html") == 0) {
    out_data = embedded_index_html;
    out_len = embedded_index_html_len;
  } else if (strcmp(path, "/style.css") == 0) {
    out_data = embedded_style_css;
    out_len = embedded_style_css_len;
  } else if (strcmp(path, "/app.js") == 0) {
    out_data = embedded_app_js;
    out_len = embedded_app_js_len;
  } else if (strcmp(path, "/joystick.js") == 0) {
    out_data = embedded_joystick_js;
    out_len = embedded_joystick_js_len;
  } else if (strcmp(path, "/websocket.js") == 0) {
    out_data = embedded_websocket_js;
    out_len = embedded_websocket_js_len;
  } else if (strcmp(path, "/ui.js") == 0) {
    out_data = embedded_ui_js;
    out_len = embedded_ui_js_len;
  } else if (strcmp(path, "/telemetry.js") == 0) {
    out_data = embedded_telemetry_js;
    out_len = embedded_telemetry_js_len;
  } else if (strcmp(path, "/camera.js") == 0) {
    out_data = embedded_camera_js;
    out_len = embedded_camera_js_len;
  } else {
    Serial.print("Not found embedded: ");
    Serial.println(path);
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file not found");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, get_content_type(path));
  httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate");
  httpd_resp_set_hdr(req, "Content-Encoding", "identity");

  Serial.print("Serve embedded: ");
  Serial.print(path);
  Serial.print(" size=");
  Serial.println(out_len);

  httpd_resp_send(req, out_data, out_len);
  return ESP_OK;
}

// --- Command timeout ---
uint32_t g_last_command_ms = 0;

void apply_movement_command(const MovementCommand &cmd) {
  if (!cmd.valid) {
    Serial.println("[UART Warning] Movement command was invalid, discarding!");
    return;
  }
  
  Serial.print("[UART] Sending to bridge: ");
  Serial.println(cmd.serial_cmd);
  
  // Forward the serial command to the Arduino Uno.
  serial_bridge_send(cmd.serial_cmd);
  g_last_command_ms = millis();
}

void handle_move(const char *payload) {
  MovementCommand cmd = movement_parse_command(payload, g_settings);
  if (cmd.valid) {
    apply_movement_command(cmd);
  }
}

void handle_settings(const char *payload) {
  char drive_val[16] = {0};
  char turn_val[16] = {0};
  if (movement_get_value(payload, "drive", drive_val, sizeof(drive_val))) {
    float drive = atof(drive_val);
    g_settings.drive = constrain(drive, 0.1f, 1.5f);
    // Forward drive sensitivity to Arduino
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "DRIVE:%.2f", g_settings.drive);
    serial_bridge_send(cmd);
  }
  if (movement_get_value(payload, "turn", turn_val, sizeof(turn_val))) {
    float turn = atof(turn_val);
    g_settings.turn = constrain(turn, 0.1f, 1.5f);
    // Forward turn sensitivity to Arduino
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "TURN:%.2f", g_settings.turn);
    serial_bridge_send(cmd);
  }
}

void handle_flashlight(const char *payload) {
  char level_val[16] = {0};
  char on_val[8] = {0};

  if (movement_get_value(payload, "level", level_val, sizeof(level_val))) {
    int level = atoi(level_val);
    level = constrain(level, 0, 255);
    flashlight_set_level(static_cast<uint8_t>(level));
  }

  if (movement_get_value(payload, "on", on_val, sizeof(on_val))) {
    flashlight_set(atoi(on_val) != 0);
  }
}

void handle_camera(const char *payload) {
  char action[16] = {0};
  if (!movement_get_value(payload, "action", action, sizeof(action))) {
    return;
  }
  if (strcmp(action, "start") == 0) {
    camera_control_set_stream_enabled(true);
  }
  if (strcmp(action, "stop") == 0) {
    camera_control_set_stream_enabled(false);
  }
}

// --- Telemetry broadcast to WebSocket clients ---

void broadcast_telemetry() {
#ifdef CONFIG_HTTPD_WS_SUPPORT
  if (!g_httpd) return;

  const BridgeTelemetry *tel = serial_bridge_get_telemetry();

  // Build the telemetry payload as URL-encoded string (same format the
  // client already understands).
  char payload[192];
  snprintf(payload, sizeof(payload),
           "type=telemetry&x=%.2f&y=%.2f&z=%.2f&a=%.2f&tilt=%s&state=%s&conn=%d",
           tel->accel_x, tel->accel_y, tel->accel_z, tel->accel_total,
           tel->tilt, tel->state, tel->connected ? 1 : 0);

  // Only broadcast if data has changed.
  if (strcmp(payload, g_last_telemetry_str) == 0) {
    return;
  }
  strncpy(g_last_telemetry_str, payload, sizeof(g_last_telemetry_str) - 1);
  g_last_telemetry_str[sizeof(g_last_telemetry_str) - 1] = '\0';

  httpd_ws_frame_t frame;
  memset(&frame, 0, sizeof(frame));
  frame.type = HTTPD_WS_TYPE_TEXT;
  frame.payload = reinterpret_cast<uint8_t *>(payload);
  frame.len = strlen(payload);

  // Send to all connected clients.
  size_t clients = config::kHttpPort;  // max fd scan range
  int fds[8];
  size_t num_fds = sizeof(fds) / sizeof(fds[0]);

  if (httpd_get_client_list(g_httpd, &num_fds, fds) == ESP_OK) {
    for (size_t i = 0; i < num_fds; i++) {
      if (httpd_ws_get_fd_info(g_httpd, fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET) {
        httpd_ws_send_frame_async(g_httpd, fds[i], &frame);
      }
    }
  }
#endif
}

esp_err_t ws_handler(httpd_req_t *req) {
#ifdef CONFIG_HTTPD_WS_SUPPORT
  if (req->method == HTTP_GET) {
    Serial.println("WebSocket Handshake");
    return ESP_OK;
  }

  httpd_ws_frame_t frame;
  memset(&frame, 0, sizeof(frame));
  frame.type = HTTPD_WS_TYPE_TEXT;

  esp_err_t res = httpd_ws_recv_frame(req, &frame, 0);
  if (res != ESP_OK) {
    return res;
  }

  if (frame.len == 0 || frame.len > 512) {
    return ESP_OK;
  }

  char *payload = static_cast<char *>(malloc(frame.len + 1));
  if (!payload) {
    return ESP_ERR_NO_MEM;
  }

  frame.payload = reinterpret_cast<uint8_t *>(payload);
  res = httpd_ws_recv_frame(req, &frame, frame.len);
  if (res != ESP_OK) {
    free(payload);
    return res;
  }
  payload[frame.len] = '\0';

  char type[16] = {0};
  if (movement_get_value(payload, "type", type, sizeof(type))) {
    // Only log non-telemetry requests to prevent console spam
    if (strcmp(type, "telemetry") != 0) {
      Serial.print("[WS] Recv: ");
      Serial.println(payload);
    }
    
    if (strcmp(type, "move") == 0) {
      Serial.println("[ROUTER] Routing to handle_move");
      handle_move(payload);
    } else if (strcmp(type, "flash") == 0) {
      Serial.println("[ROUTER] Routing to handle_flashlight");
      handle_flashlight(payload);
    } else if (strcmp(type, "camera") == 0) {
      Serial.println("[ROUTER] Routing to handle_camera");
      handle_camera(payload);
    } else if (strcmp(type, "settings") == 0) {
      Serial.println("[ROUTER] Routing to handle_settings");
      handle_settings(payload);
    }
  } else {
    Serial.print("[WS Warning] Missing 'type' in payload: ");
    Serial.println(payload);
  }

  free(payload);
  return ESP_OK;
#else
  (void)req;
  return ESP_ERR_NOT_SUPPORTED;
#endif
}
}  // namespace

bool websocket_handler_start_server() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = config::kHttpPort;
  config.max_uri_handlers = 8;
  config.uri_match_fn = httpd_uri_match_wildcard;

  httpd_uri_t file_uri = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = file_handler,
    .user_ctx = NULL
#ifdef CONFIG_HTTPD_WS_SUPPORT
    ,
    .is_websocket = false,
    .handle_ws_control_frames = false,
    .supported_subprotocol = NULL
#endif
  };

  httpd_uri_t ws_uri = {
    .uri = config::kWsPath,
    .method = HTTP_GET,
    .handler = ws_handler,
    .user_ctx = NULL
#ifdef CONFIG_HTTPD_WS_SUPPORT
    ,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = NULL
#endif
  };

  if (httpd_start(&g_httpd, &config) != ESP_OK) {
    return false;
  }

  httpd_register_uri_handler(g_httpd, &ws_uri);
  httpd_register_uri_handler(g_httpd, &file_uri);
  return true;
}

void websocket_handler_tick(uint32_t now_ms) {
  // Command timeout safety: if no movement command received recently,
  // send a STOP to the Arduino as a safety net.
  static bool s_timeout_sent = false;

  if (now_ms - g_last_command_ms > config::kCommandTimeoutMs) {
    // Only send once after timeout expires, not every tick.
    if (!s_timeout_sent) {
      serial_bridge_send("MOVE:S");
      s_timeout_sent = true;
    }
  } else {
    // We're within the active window — allow the next timeout to fire.
    s_timeout_sent = false;
  }

  // Broadcast telemetry to WebSocket clients periodically.
  if (now_ms - g_last_telemetry_broadcast_ms > config::kTelemetryBroadcastMs) {
    g_last_telemetry_broadcast_ms = now_ms;
    broadcast_telemetry();
  }
}
