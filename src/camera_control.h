#pragma once

#include <Arduino.h>
#include "esp_http_server.h"

bool camera_control_init();
void camera_control_start_stream_server(uint16_t port);
void camera_control_set_stream_enabled(bool enabled);
bool camera_control_is_stream_enabled();
