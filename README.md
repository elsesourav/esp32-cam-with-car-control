# ESP32-CAM Standalone WiFi RC Car 🏎️📹

A professional, single-board WiFi car control system with a modern, mobile-friendly web UI. 
This project has been completely refactored to remove the Arduino Uno dependency. The ESP32-CAM now acts as the sole brain: handling the web server, real-time video streaming, WebSocket communication, direct PWM motor control, and direct I2C MPU6050 telemetry.

**Developed by:** [SouravBarui](https://github.com/elsesourav)

---

## 🚀 Features

- **Standalone Architecture**: No secondary microcontroller needed. Drives L298N and reads MPU6050 directly.
- **4-Pin PWM Trick**: Innovative use of L298N IN pins for PWM to save precious ESP32-CAM GPIOs.
- **Proportional Joystick UI**: Mobile-first touch controller (HTML/CSS/JS) with dark glassmorphism design.
- **Live Camera Stream**: Stream MJPEG video directly to your phone/computer.
- **Embedded Web Server**: No SPIFFS/LittleFS plugins required! All website assets are safely embedded and served from memory.
- **Live Telemetry**: Real-time display of MPU6050 accelerometer and tilt data in the web UI.
- **Flashlight Dimmer**: PWM-controlled flashlight UI slider.

---

## 🔌 Hardware Wiring Guide

The ESP32-CAM has very few available external pins. We use a specific wiring layout to avoid boot conflicts while retaining full motor control and sensor support.

### 1. Motors (L298N Driver)
> **CRITICAL**: Do **NOT** remove the black plastic jumpers on the ENA and ENB pins of your L298N. Leave them ON so the motors are always enabled. The ESP32-CAM applies PWM directly to the IN1-IN4 pins to control both speed and direction.

| ESP32-CAM Pin | L298N Pin | Function | Notes |
|---|---|---|---|
| **GPIO 12** | **IN1** | Left Motor Forward | Must be LOW at boot (Safe). |
| **GPIO 13** | **IN2** | Left Motor Backward | Safe pin. |
| **GPIO 2** | **IN3** | Right Motor Forward | Must be LOW for flashing (Safe). |
| **GPIO 3 (RX)** | **IN4** | Right Motor Backward | **UNPLUG THIS WIRE WHEN FLASHING CODE.** |
| **GND** | **GND** | Common Ground | MUST share ground with ESP32-CAM! |

### 2. Telemetry (MPU6050 Sensor)
The MPU6050 shares the internal I2C bus with the OV2640 camera using pins 14 and 15.

| ESP32-CAM Pin | MPU6050 Pin | Function |
|---|---|---|
| **GPIO 14** | **SDA** | I2C Data |
| **GPIO 15** | **SCL** | I2C Clock (Safe for pull-up) |
| **3.3V** | **VCC** | 3.3V Power |
| **GND** | **GND** | Common Ground |

---

## 🛠️ Setup & Installation Guide

### Step 1 — Install Arduino IDE & ESP32 Core

1. Download and install [Arduino IDE (2.x)](https://www.arduino.cc/en/software).
2. Open Arduino IDE and go to **Arduino → Settings**.
3. Add this URL to **Additional boards manager URLs**:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Go to **Tools → Board → Boards Manager**, search for `esp32` and click **Install**.

### Step 2 — Configure Project

1. Open `esp32-cam-with-car-control.ino` in Arduino IDE.
2. Open `src/config.h` and update your WiFi credentials:
   ```cpp
   constexpr char kWifiSsid[] = "Your_WiFi_SSID";
   constexpr char kWifiPass[] = "Your_WiFi_PASSWORD";
   ```

### Step 3 — Select ESP32-CAM Board Settings

Go to **Tools** and configure exactly like this:
- **Board**: `AI Thinker ESP32-CAM`
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Mode**: `QIO`
- **Partition Scheme**: **`Huge APP (3MB No OTA / 1MB SPIFFS)`** *(Critical for fitting camera + web server).*

### Step 4 — Upload to ESP32-CAM

> ⚠️ **FLASHING WARNING:** You are using GPIO 3 (RX) for the Right Motor. You **MUST** unplug the wire from GPIO 3 on the ESP32-CAM before attempting to flash code, or the upload will fail.

1. Unplug the wire from `GPIO 3` (if connected).
2. Connect `IO0` (GPIO 0) to `GND` to place the board in bootloader mode.
3. Hit the Reset button on the ESP32-CAM.
4. Click **Upload** in the Arduino IDE.
5. Once complete, disconnect `IO0` from `GND`.
6. Plug the motor wire back into `GPIO 3`.
7. Hit the Reset button to boot normally.

### Step 5 — Drive!

1. Open the **Serial Monitor** (115200 baud).
2. Wait for the `WiFi connected, IP: XXX.XXX.X.X` line.
3. Open that IP address in your browser (e.g., http://10.68.70.248).
4. Tap **Start Camera** and start driving!

---

## 🚨 Common Troubleshooting

- **Upload Failed (Timeout):** Did you remember to unplug the motor wire from GPIO 3 (RX)? Is GPIO 0 connected to GND? Did you press Reset?
- **Camera Init Failed:** Check if the camera ribbon cable is seated properly. Ensure you selected the `Huge APP` partition scheme.
- **ESP32 Keeps Restarting (Brownout):** The motors are drawing too much power and crashing the ESP32. Provide a separate strong 5V power supply for the L298N and ESP32-CAM, and ensure all GNDs are connected.
- **Motors Don't Move:** Did you leave the black jumpers ON the ENA and ENB pins of the L298N? If you took them off, the motors won't run.

---

## 📁 System Architecture

- **`esp32-cam-with-car-control.ino`** - Entry point, starts WiFi, camera, motors, MPU.
- **`src/` (ESP32-CAM Backend)**
  - `config.h` - Pin mapping and network config.
  - `motor_control.*` - Direct L298N PWM control via ESP32 LEDC.
  - `mpu6050_sensor.*` - Direct I2C telemetry polling.
  - `movement_parser.*` - JSON/URL string math conversion.
  - `camera_control.*` - HTTP stream setup.
  - `websocket_handler.*` - Runs the server, serves UI, handles WebSockets.
  - `embedded_files.h` - Bundled C++ arrays for the UI (created by `create_embedded.js`).
- **`data/` (Web UI)**
  - Contains `index.html`, `style.css`, and `.js` files. Run `node src/create_embedded.js` to update the header after editing.

---

## 📝 License

This project is open-source and released under the **MIT License**. Check the [LICENSE](LICENSE) file for more details.
