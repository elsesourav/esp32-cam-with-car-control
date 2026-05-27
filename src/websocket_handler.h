#pragma once

#include <Arduino.h>

bool websocket_handler_start_server();
void websocket_handler_tick(uint32_t now_ms);
