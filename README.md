# ESP32-CAM WiFi RC Car 🏎️📹

A professional ESP32-CAM WiFi car control system with a modern, mobile-friendly web UI.
Drive via an on-screen joystick, stream real-time video, and toggle the board's flashlight—all embedded within a single fast ESP32 setup!

**Developed by:** [SouravBarui](https://github.com/elsesourav)

---

## 🚀 Features

- **Differential Drive Control**: Smooth proportional control using dual DC motors (L298N driver).
- **Proportional Joystick UI**: Mobile-first touch controller (HTML/CSS/JS).
- **Live Camera Stream**: Stream MJPEG video directly to your phone/computer.
- **Embedded Web Server**: No SPIFFS/LittleFS plugins required! All website assets are safely embedded and served from memory.
- **Flashlight Dimmer**: PWM-controlled flashlight UI slider.

---

## 🛠️ Detailed Setup Guide

### Step 1 — Hardware Requirements

- **ESP32-CAM** (AI Thinker module)
- **FTDI Programmer** (to upload the code)
- **L298N Motor Driver**
- **2x DC Motors & Smart Car Chassis**
- **Power Supply** (e.g., 2x 18650 batteries)

### Step 2 — Install Arduino IDE & ESP32 Core

1. Download and install [Arduino IDE (2.x)](https://www.arduino.cc/en/software).
2. Open Arduino IDE and go to **Arduino → Settings** (`⌘ + ,` or `Ctrl + ,`).
3. Add the following URL to **Additional boards manager URLs**:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Open the **Tools → Board → Boards Manager**, search for `esp32` and click **Install**.

### Step 3 — Configure Project

1. Clone / Download this repository.
2. Open `EST32_Using_WiFi.ino` in Arduino IDE.
3. Open `src/config.h` and update your WiFi credentials:
   ```cpp
   constexpr char kWifiSsid[] = "Your_WiFi_SSID";
   constexpr char kWifiPass[] = "Your_WiFi_PASSWORD";
   ```

### Step 4 — Select Board Settings

Go to **Tools** and configure the board exactly like this:

- **Board**: `AI Thinker ESP32-CAM`
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Frequency**: `80MHz`
- **Flash Mode**: `QIO`
- **Partition Scheme**: **`Huge APP (3MB No OTA / 1MB SPIFFS)`** _(Strong Recommendation: You must select a larger partition to fit the firmware, HTML UI, and camera libraries)._
- **Port**: Select your FTDI USB serial port.

### Step 5 — Upload the Code

_Note: Because we use an embedded PROGMEM bundle for the data, you do **NOT** need the LittleFS/SPIFFS data upload plugin anymore! Just a standard upload._

1. Connect `IO0` (GPIO 0) to `GND` on your ESP32-CAM to place it in bootloader mode.
2. Hit the Reset button on the ESP32-CAM.
3. Click the **Upload** button in the Arduino IDE.
4. Once the upload reads **Done uploading**, disconnect `IO0` from `GND`.
5. Hit the **Reset** button again to boot normally.

### Step 6 — Connect & Drive

1. Open the **Serial Monitor** (set baud rate to `115200`).
2. Wait for the `WiFi connected, IP: XXX.XXX.X.X` line.
3. Open that IP address in your browser (e.g., http://10.68.70.248) to load the UI!
4. Tap the **Start Camera** button to enable the live stream.

---

## 📁 System Architecture

- **`EST32_Using_WiFi.ino`** - Entry point, starts WiFi / motor / camera systems.
- **`src/`**
  - `config.h` - Network and global pins.
  - `motor_control.*` / `movement_parser.*` - PWM conversion and joystick math.
  - `camera_control.*` - ESP-CAM initialization and HTTP stream setup.
  - `websocket_handler.*` - Runs the ESP32 HTTPD server, serves UI from memory, and brokers the control WebSocket.
  - `embedded_files.h` - Contains the bundled C++ arrays for the UI (created by `create_embedded.js`).
- **`data/`**
  - Contains the raw `index.html`, `style.css`, and `.js` source files. (Run `node src/create_embedded.js` if you ever edit these to update the header).

---

## 📝 License

This project is open-source and released under the **MIT License**. Check the [LICENSE](LICENSE) file for more details.
