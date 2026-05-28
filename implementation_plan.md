# Dual-Board Architecture: ESP32-CAM + Arduino Uno

Refactoring the existing single-board ESP32-CAM car control project into a dual-board system where the ESP32-CAM handles WiFi/camera/UI and an Arduino Uno handles motors/sensors. The existing dark futuristic UI design is fully preserved.

## User Review Required

> [!IMPORTANT]
> **UART Wiring**: The ESP32-CAM runs at 3.3V logic levels while the Arduino Uno runs at 5V. You **must** use a voltage level shifter (or a simple resistor divider) on the `Arduino TX → ESP32 RX` line to avoid damaging the ESP32. The `ESP32 TX → Arduino RX` line is safe because 3.3V is high enough for the Uno to read as HIGH.

> [!WARNING]
> **Boot Conflict**: ESP32-CAM GPIO 1 (TX0) and GPIO 3 (RX0) are the default UART0 pins used for flashing/Serial Monitor. We will use **UART2** on GPIO 14 (TX) and GPIO 15 (RX) for the Arduino bridge instead, leaving UART0 free for debugging. This means GPIO 14 and 15 are now reserved for serial communication and **cannot** be used for motors.

## Physical Wiring Diagram

### UART Connection (ESP32-CAM ↔ Arduino Uno)
```
ESP32-CAM GPIO 14 (TX2)  →  Arduino Uno Pin 10 (SoftwareSerial RX)
ESP32-CAM GPIO 15 (RX2)  ←  Arduino Uno Pin 11 (SoftwareSerial TX) [via level shifter]
ESP32-CAM GND             ↔  Arduino Uno GND (COMMON GROUND REQUIRED)
```
- **Baud rate**: 115200
- ESP32-CAM uses HardwareSerial (Serial2) on pins 14/15
- Arduino Uno uses SoftwareSerial on pins 10/11

### L298N Motor Driver #1 (Left Side)
```
Arduino Uno Pin 2   →  IN1 (Left Forward)
Arduino Uno Pin 4   →  IN2 (Left Backward)
Arduino Uno Pin 3   →  ENA (Left PWM Speed) [must be PWM pin]
```

### L298N Motor Driver #2 (Right Side)
```
Arduino Uno Pin 7   →  IN3 (Right Forward)
Arduino Uno Pin 8   →  IN4 (Right Backward)
Arduino Uno Pin 5   →  ENB (Right PWM Speed) [must be PWM pin]
```

### MPU6050 (I2C to Arduino Uno)
```
MPU6050 SDA  →  Arduino Uno A4 (SDA)
MPU6050 SCL  →  Arduino Uno A5 (SCL)
MPU6050 VCC  →  Arduino Uno 5V
MPU6050 GND  →  Arduino Uno GND
```

---

## Serial Protocol

### ESP32 → Arduino (Commands)
| Command | Format | Description |
|---------|--------|-------------|
| Move direction | `MOVE:F\n` | F, B, L, R, FL, FR, BL, BR, S (stop) |
| Differential | `DIFF:120,-120\n` | Left PWM, Right PWM (for joystick) |
| Speed | `SPD:180\n` | Set base speed (0-255) |
| Turn ratio | `TURN:1.20\n` | Set turn sensitivity multiplier |
| Drive ratio | `DRIVE:1.00\n` | Set drive sensitivity multiplier |

### Arduino → ESP32 (Telemetry)
| Response | Format | Description |
|----------|--------|-------------|
| MPU data | `MPU:0.12,-0.45,9.80,1.23,FLAT\n` | X, Y, Z accel, total accel, tilt direction |
| State | `STATE:FORWARD\n` | Current movement state |
| Status | `STATUS:OK\n` | Heartbeat/health |

---

## Proposed Changes

### ESP32-CAM Project — Files to Delete
These files are no longer needed since motor control moves to the Arduino:

#### [DELETE] [motor_control.h](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/motor_control.h)
#### [DELETE] [motor_control.cpp](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/motor_control.cpp)

---

### ESP32-CAM Project — Config

#### [MODIFY] [config.h](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/config.h)
- Remove all motor pin definitions (kMotorA/B/C/D)
- Remove motor PWM frequency/resolution constants
- Add UART2 bridge config: `kBridgeTx = 14`, `kBridgeRx = 15`, `kBridgeBaud = 115200`
- Keep WiFi, HTTP, stream, flashlight config unchanged

---

### ESP32-CAM Project — New Serial Bridge Module

#### [NEW] serial_bridge.h + serial_bridge.cpp
- `serial_bridge_init()` — configures HardwareSerial (Serial2) on GPIO 14/15 at 115200 baud
- `serial_bridge_send(const char* cmd)` — sends a command string to the Arduino (non-blocking)
- `serial_bridge_tick()` — reads incoming telemetry from Arduino, parses `MPU:`, `STATE:`, `STATUS:` lines
- Stores latest telemetry in a struct: `{ float x, y, z, accel; char tilt[16]; char state[16]; bool connected; }`
- `serial_bridge_get_telemetry()` — returns pointer to latest telemetry struct

---

### ESP32-CAM Project — Movement Parser Refactor

#### [MODIFY] [movement_parser.cpp](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/movement_parser.cpp)
- Keep the same WebSocket payload parsing logic (it still works)
- Instead of outputting `left_speed`/`right_speed` for local motors, the `MovementCommand` struct gains a `char serial_cmd[32]` field containing the formatted serial command to send to the Arduino

#### [MODIFY] [movement_parser.h](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/movement_parser.h)
- Add `char serial_cmd[32]` to `MovementCommand` struct

---

### ESP32-CAM Project — WebSocket Handler Refactor

#### [MODIFY] [websocket_handler.cpp](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/websocket_handler.cpp)
- Replace `motor_control_*` calls with `serial_bridge_send()` calls
- Instead of calling `motor_control_set_differential()`, format and send `DIFF:left,right` to the Arduino
- Add a new WebSocket message type `telemetry` — on each tick, if telemetry has changed, broadcast it to connected WebSocket clients
- Remove `#include "motor_control.h"`, add `#include "serial_bridge.h"`

#### [MODIFY] [websocket_handler.h](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/websocket_handler.h)
- No API change needed

---

### ESP32-CAM Project — Main INO

#### [MODIFY] [esp32-cam-with-car-control.ino](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/esp32-cam-with-car-control.ino)
- Remove `motor_control_init()` call
- Add `serial_bridge_init()` call
- Add `serial_bridge_tick()` in `loop()`
- Keep all other init (WiFi, camera, flashlight, websocket) unchanged

---

### ESP32-CAM Project — Embedded Files Generator

#### [MODIFY] [create_embedded.js](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/src/create_embedded.js)
- Add `telemetry.js` and `camera.js` to the files list

---

### ESP32-CAM Project — Web UI (data/)

#### [MODIFY] [index.html](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/index.html)
- Add a new `<section class="telemetry glass">` at the bottom of `<main>` with MPU6050 telemetry display (X, Y, Z, acceleration, tilt, movement state, connection indicator)
- Add `<script type="module" src="/telemetry.js">` — but actually all JS is loaded from `app.js` as modules, so just import it there
- Keep ALL existing HTML structure unchanged

#### [MODIFY] [style.css](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/style.css)
- Add `.telemetry` section styles using the existing dark glassmorphism design language
- Add `.telemetry-grid` for the 2-column grid of sensor values
- Add `.telemetry-value` monospace badge styling (matching `.val-badge`)
- Add `.sensor-dot` connection indicator (matching `.dot` style)
- Keep ALL existing CSS unchanged

#### [NEW] telemetry.js
- Exports `initTelemetry(elements)` and `updateTelemetry(elements, data)`
- Binds to DOM elements for X, Y, Z, accel, tilt, state, connection dot
- Called from `app.js` when WebSocket receives telemetry messages

#### [NEW] camera.js
- Extracts camera start/stop/stream logic from `app.js` into its own module
- Exports `initCamera(elements, wsClient)` with start/stop/stream functions

#### [MODIFY] [app.js](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/app.js)
- Add `import` for `telemetry.js`
- Add WebSocket `onMessage` handler that parses incoming telemetry data and updates the UI
- Extract camera logic into `camera.js` import
- Keep all existing joystick, button, slider, popup logic unchanged

#### [MODIFY] [ui.js](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/ui.js)
- Add telemetry DOM elements to `initUI()` (telemetry value spans, sensor dot)
- Keep all existing functions unchanged

#### [MODIFY] [websocket.js](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/websocket.js)
- Add `onMessage` callback to `WsClient` class so telemetry data can be received
- Keep all existing send/reconnect logic unchanged

#### No changes needed to: [joystick.js](file:///Users/sourav/Documents/Arduino/esp32-cam-with-car-control/data/joystick.js)

---

### Arduino Uno Project — Entirely New

#### [NEW] `/arduino_uno_controller/` — Complete separate Arduino project

| File | Purpose |
|------|---------|
| `arduino_uno_controller.ino` | Main sketch: init motors, MPU, serial; loop reads commands and sends telemetry |
| `config.h` | Pin definitions, baud rate, PWM limits, telemetry interval |
| `serial_protocol.h / .cpp` | SoftwareSerial setup, command parser, telemetry sender |
| `motor_controller.h / .cpp` | L298N driver: `init()`, `setDifferential(left, right)`, `stop()`, direction via IN1/IN2 |
| `pwm_controller.h / .cpp` | Smooth PWM ramping/acceleration for gradual speed changes |
| `movement_parser.h / .cpp` | Parses `MOVE:`, `DIFF:`, `SPD:`, `TURN:`, `DRIVE:` commands from serial |
| `mpu6050_handler.h / .cpp` | I2C MPU6050 reading with simple complementary filter, tilt calculation |

---

## Open Questions

> [!IMPORTANT]
> 1. **Level Shifter**: Do you have a 3.3V ↔ 5V logic level shifter module? If not, a simple resistor divider (1kΩ + 2kΩ) on the Arduino TX line will work.
> 2. **MPU6050 Library**: Should I use the raw I2C register reads (no external library needed) or the Adafruit MPU6050 library? Raw reads save Flash memory on the Uno.
> 3. **SoftwareSerial Pins**: I chose Arduino pins 10/11 for SoftwareSerial. Do you have any other devices on those pins?

---

## Verification Plan

### Automated Tests
- Compile ESP32-CAM project with Arduino CLI for `esp32:esp32:esp32cam`
- Compile Arduino Uno project with Arduino CLI for `arduino:avr:uno`

### Manual Verification
1. Flash ESP32-CAM → verify WiFi connects, camera streams, web UI loads with telemetry section
2. Flash Arduino Uno → verify serial monitor shows `STATUS:OK` heartbeat
3. Wire UART between boards → verify movement commands flow through and telemetry appears in the web UI
4. Test joystick mode → verify smooth differential drive through serial bridge
5. Test button mode → verify direction lock behavior preserved
6. Test flashlight slider → verify PWM brightness still works on ESP32-CAM
