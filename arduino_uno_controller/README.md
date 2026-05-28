# Arduino Uno Motor & Sensor Controller

This directory contains the firmware for the Arduino Uno, which acts as the real-time hardware controller in the ESP32-CAM dual-board architecture.

## Responsibilities
- **Motor Control**: Drives 2x L298N motor drivers for differential steering.
- **PWM Smoothing**: Implements gradual acceleration/deceleration to prevent voltage spikes.
- **Sensor Reading**: Interfaces with the MPU6050 accelerometer/gyroscope over I2C.
- **Serial Communication**: Receives commands from the ESP32-CAM and sends back telemetry over a 115200 baud UART bridge.
- **Safety**: Automatically stops motors if communication with the ESP32-CAM is lost.

## Hardware Connections

### MPU6050
- `VCC` -> `5V`
- `GND` -> `GND`
- `SDA` -> `A4`
- `SCL` -> `A5`

### ESP32-CAM (UART Bridge)
- `Pin 10 (RX)` -> `ESP32-CAM GPIO 14 (TX)`
- `Pin 11 (TX)` -> `ESP32-CAM GPIO 15 (RX)` *(Important: Use a logic level shifter or voltage divider. ESP32 is 3.3V, Uno is 5V)*
- `GND` -> `ESP32-CAM GND`

### L298N Motor Driver 1 (Left Side)
- `IN1` -> `Pin 2`
- `IN2` -> `Pin 4`
- `ENA` -> `Pin 3 (PWM)`

### L298N Motor Driver 2 (Right Side)
- `IN3` -> `Pin 7`
- `IN4` -> `Pin 8`
- `ENB` -> `Pin 5 (PWM)`

## Uploading
1. Open `arduino_uno_controller.ino` in the Arduino IDE.
2. Select **Tools -> Board -> Arduino Uno**.
3. Select your COM port.
4. Click **Upload**.
