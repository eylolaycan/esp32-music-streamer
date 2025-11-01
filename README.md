## ESP32 Music Streamer (Standalone BLE Music Player)

An embedded audio player project based on the ESP32 microcontroller.
The system hosts its own Wi-Fi web server that allows users to control audio playback directly from a browser.
MP3 files are stored locally on an SD card, and audio is streamed over Bluetooth to a connected device.

## Features

- Standalone ESP32-based MP3 player
- Local file storage using SD card
- Web interface served directly from the ESP32
- Audio output via Bluetooth (A2DP)
- SSD1306 OLED display for track information and playback status
- Physical buttons for play, pause, next, previous, and volume control
- PCB design planned (currently operating on breadboard)
- No enclosure yet (hardware is fully exposed)

## Technologies and Tools

- Microcontroller: ESP32 (ESP-IDF Framework)
- File System: SD card (FAT32)
- User Interface: HTML/CSS web interface hosted on ESP32
- Display: SSD1306 OLED (I²C communication)
- Programming Language: C++
- PCB Design Tool: Altium Designer (planned for final version)
