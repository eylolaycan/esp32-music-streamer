# 🎵 ESP32 Music Streamer (Standalone Wi-Fi Audio Player)

An embedded audio player based on ESP32 that hosts its own Wi-Fi server. Users can connect via browser to control audio playback. MP3 files are stored locally (SPIFFS or SD card), and sound is played through an I2S DAC module (e.g., PCM5102A or MAX98357A).

---

## 🚀 Features

- ✅ Standalone ESP32-based MP3 player (no Bluetooth needed)
- ✅ Local file storage via **SPIFFS** or **SD card**
- ✅ Web interface hosted directly on ESP32 (Access Point mode)
- ✅ Audio output through **I2S DAC** (e.g., PCM5102A, MAX98357A)
- ✅ SSD1306 OLED shows track info and playback status
- ✅ Physical buttons for play/pause, next/previous, volume
- ☑️ *PCB design not yet started – currently on breadboard*
- ☑️ *No 3D printed case – hardware is exposed*

---

## 🧰 Technologies & Tools

- **Microcontroller**: ESP32 (ESP-IDF Framework)
- **Audio**: I2S DAC (MAX98357A, PCM5102A)
- **File System**: SPIFFS or SD card
- **User Interface**: HTML/CSS (served from ESP32)
- **Display**: SSD1306 OLED (I2C)
- **Programming**: C++
- **PCB Design**: *(Planned – to be done with Altium Designer)*

---

## 🗂 Project Structure

esp32-music-streamer/
├── main/
│ ├── app_main.cpp # Initialization and app entry
│ ├── file_player.cpp # MP3 decoding and I2S output
│ ├── web_server.cpp # HTTP server and web UI
│ ├── display.cpp # OLED control
│ └── buttons.cpp # GPIO button handling
│
├── spiffs/ # Folder to upload to SPIFFS (MP3 files, web UI)
├── include/ # Header files
├── CMakeLists.txt
└── sdkconfig


## 🔧 Setup & Flashing

### Prerequisites

- ESP-IDF v4.4+ installed
- Supported ESP32 dev board
- I2S DAC module connected
- (Optional) SSD1306 OLED screen connected via I2C

### Steps

```bash
git clone https://github.com/eylolaycan/esp32-music-streamer.git
cd esp32-music-streamer

idf.py set-target esp32
idf.py menuconfig       # Choose SPIFFS or SD card, set pinout, Wi-Fi settings
idf.py build
idf.py flash monitor
