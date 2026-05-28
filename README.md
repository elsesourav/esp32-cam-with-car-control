# ESP32-CAM WiFi RC Car 🏎️📹

A professional WiFi car control system with a modern, mobile-friendly web UI. 
This project focuses on the ESP32-CAM which handles the web server, real-time video streaming, and WebSocket communication. It acts as the brain, sending movement commands and receiving telemetry via a UART bridge to an Arduino Uno (which handles the low-level motor and sensor hardware).

**Developed by:** [SouravBarui](https://github.com/elsesourav)

---

## 🚀 Features

- **Proportional Joystick UI**: Mobile-first touch controller (HTML/CSS/JS) with dark glassmorphism design.
- **Live Camera Stream**: Stream MJPEG video directly to your phone/computer.
- **Embedded Web Server**: No SPIFFS/LittleFS plugins required! All website assets are safely embedded and served from memory.
- **UART Bridge**: Seamlessly communicates with a secondary board (Arduino Uno) to offload motor control and sensor parsing.
- **Live Telemetry**: Real-time display of MPU6050 accelerometer and tilt data in the web UI.
- **Flashlight Dimmer**: PWM-controlled flashlight UI slider.

---

## 🔌 ESP32-CAM Pin Connections

The ESP32-CAM requires a serial connection to the Arduino Uno to transmit commands and receive telemetry data.

| ESP32-CAM Pin | Connection | Description |
|---|---|---|
| **GPIO 14 (TX2)** | **Arduino Uno Pin 10 (RX)** | Transmits commands (MOVE, DIFF, SPD, etc.) to the Uno. |
| **GPIO 15 (RX2)** | **Arduino Uno Pin 11 (TX)** | Receives telemetry and status from the Uno. *(Note: Recommend using a logic level shifter here since ESP32 is 3.3V and Uno is 5V).* |
| **GND** | **Arduino Uno GND** | **Critical:** Both boards must share a common ground. |
| **GPIO 4** | Built-in Flashlight | Used for the high-power onboard LED. |
| **5V / 3.3V** | Power Supply | Depends on your specific power setup. |

> ⚠️ **Looking for Motor/Sensor wiring?** 
> Motor drivers (L298N) and the MPU6050 sensor are connected directly to the Arduino Uno. Please see the [arduino_uno_controller/README.md](arduino_uno_controller/README.md) for full hardware wiring and setup instructions for the Uno side.

---

## 🛠️ ESP32-CAM Setup Guide

### Step 1 — Install Arduino IDE & ESP32 Core

1. Download and install [Arduino IDE (2.x)](https://www.arduino.cc/en/software).
2. Open Arduino IDE and go to **Arduino → Settings** (`⌘ + ,` or `Ctrl + ,`).
3. Add the following URL to **Additional boards manager URLs**:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Open the **Tools → Board → Boards Manager**, search for `esp32` and click **Install**.

### Step 2 — Configure ESP32-CAM Project

1. Open `esp32-cam-with-car-control.ino` in Arduino IDE.
2. Open `src/config.h` and update your WiFi credentials:
   ```cpp
   constexpr char kWifiSsid[] = "Your_WiFi_SSID";
   constexpr char kWifiPass[] = "Your_WiFi_PASSWORD";
   ```

### Step 3 — Select ESP32-CAM Board Settings

Go to **Tools** and configure the board exactly like this:

- **Board**: `AI Thinker ESP32-CAM`
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Frequency**: `80MHz`
- **Flash Mode**: `QIO`
- **Partition Scheme**: **`Huge APP (3MB No OTA / 1MB SPIFFS)`** _(Strong Recommendation: You must select a larger partition to fit the firmware, HTML UI, and camera libraries)._
- **Port**: Select your FTDI USB serial port.

### Step 4 — Upload to ESP32-CAM

_Note: Because we use an embedded PROGMEM bundle for the data, you do **NOT** need the LittleFS/SPIFFS data upload plugin anymore! Just a standard upload._

1. Connect `IO0` (GPIO 0) to `GND` on your ESP32-CAM to place it in bootloader mode.
2. Hit the Reset button on the ESP32-CAM.
3. Click the **Upload** button in the Arduino IDE.
4. Once the upload reads **Done uploading**, disconnect `IO0` from `GND`.
5. Hit the **Reset** button again to boot normally.

*(Make sure you also upload the [Arduino Uno firmware](arduino_uno_controller/README.md) to your Uno!)*

### Step 5 — Connect & Drive

1. Connect the ESP32-CAM to the Arduino Uno via the UART pins mentioned above.
2. Open the **Serial Monitor** (set baud rate to `115200`).
3. Wait for the `WiFi connected, IP: XXX.XXX.X.X` line.
4. Open that IP address in your browser (e.g., http://10.68.70.248) to load the UI!
5. Tap the **Start Camera** button to enable the live stream.

---

## 📁 System Architecture

- **`esp32-cam-with-car-control.ino`** - Entry point for ESP32-CAM, starts WiFi / camera / UART bridge.
- **`src/` (ESP32-CAM Backend)**
  - `config.h` - Network and UART pins.
  - `serial_bridge.*` - UART communication with the Arduino Uno.
  - `movement_parser.*` - Serial command formatting.
  - `camera_control.*` - ESP-CAM initialization and HTTP stream setup.
  - `websocket_handler.*` - Runs the ESP32 HTTPD server, serves UI from memory, brokers the control WebSocket, and broadcasts telemetry.
  - `embedded_files.h` - Contains the bundled C++ arrays for the UI (created by `create_embedded.js`).
- **`data/` (Web UI)**
  - Contains the raw `index.html`, `style.css`, and modular `.js` files. (Run `node src/create_embedded.js` if you ever edit these to update the header).
- **`arduino_uno_controller/`** - See [arduino_uno_controller/README.md](arduino_uno_controller/README.md) for Uno-specific details.

---

## 📝 License

This project is open-source and released under the **MIT License**. Check the [LICENSE](LICENSE) file for more details.
