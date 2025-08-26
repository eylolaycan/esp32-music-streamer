# 🎵 ESP32 Music Streamer (Standalone BLE Music Player)

An embedded audio player based on ESP32 that hosts its own Wi-Fi server. Users can connect via browser to control audio playback. MP3 files are stored locally on SD card, and sound is played through bluetooth.
---

## 🚀 Features

- ✅ Standalone ESP32-based MP3 player
- ✅ Local file storage via **SD card**
- ✅ Web interface hosted directly on ESP32 
- ✅ Audio output through BLE
- ✅ SSD1306 OLED shows track info and playback status
- ✅ Physical buttons for play/pause, next/previous, volume
- ☑️ *PCB design not yet started – currently on breadboard*
- ☑️ *No 3D printed case – hardware is exposed*

---

## 🧰 Technologies & Tools

- **Microcontroller**: ESP32 (ESP-IDF Framework)
- **File System**: SD card
- **User Interface**: HTML/CSS (served from ESP32)
- **Display**: SSD1306 OLED (I2C)
- **Programming**: C++
- **PCB Design**: *(Planned – to be done with Altium Designer)*

---

## 🗂 Project Structure-

## 🔧 Setup & Flashing

### Prerequisites

### Steps

```bash
git clone https://github.com/eylolaycan/esp32-music-streamer.git
cd esp32-music-streamer

idf.py set-target esp32
idf.py menuconfig       # Choose SPIFFS or SD card, set pinout, Wi-Fi settings
idf.py build
idf.py flash monitor
