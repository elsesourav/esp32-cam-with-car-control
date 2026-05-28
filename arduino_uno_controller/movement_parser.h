#pragma once

#include <Arduino.h>

// Parse a command line received from the ESP32-CAM.
// Supports: MOVE:F, DIFF:120,-120, SPD:180, TURN:1.20, DRIVE:1.00
// Returns true if the command was valid and applied.
bool movement_parser_handle(const char *line);

// Get the current base speed setting (set by SPD: command).
int movement_parser_get_speed();

// Get the current turn sensitivity (set by TURN: command).
float movement_parser_get_turn();

// Get the current drive sensitivity (set by DRIVE: command).
float movement_parser_get_drive();
