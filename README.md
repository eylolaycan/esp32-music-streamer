# ESP32-Based Portable Audio System – Schematic Overview

This repository contains the schematic design of a **battery-powered embedded audio system** based on the **ESP32-WROOM-1** module.  
The system integrates **power management, digital storage, audio processing, and headphone output** in a modular **mixed-signal architecture** optimized for **low-power portable audio applications**.

---

## 1. System Architecture

The design is composed of the following functional blocks:

- USB-C power input  
- Switching regulator (5 V → 3.3 V)  
- Li-ion battery charger with I²C control  
- ESP32 main controller  
- External SPI NOR flash memory  
- I²S digital-to-analog converter (DAC)  
- Stereo headphone amplifier + TRS headphone output  

---

## 2. USB-C Power Input

The USB-C connector provides a **5 V VBUS supply**.

- CC1 and CC2 pins use **5.1 kΩ pull-down resistors**, configuring the device as a **USB sink**
- VBUS directly supplies the buck regulator and battery charger

\[
V_{VBUS} \approx 5\,\text{V}
\]

---

## 3. Switching Regulator (3.3 V Rail)

A buck converter generates the main **3.3 V system rail**.

For an ideal buck converter:

\[
V_{OUT} = D \cdot V_{IN}
\]

With \(V_{IN}=5\,\text{V}\) and \(V_{OUT}=3.3\,\text{V}\):

\[
D \approx \frac{3.3}{5} \approx 0.66
\]

This rail powers:

- ESP32
- NOR flash
- DAC (digital + analog supplies)
- Audio amplifier logic

---

## 4. ESP32 Core Subsystem

The **ESP32-WROOM-1** acts as the main controller.

### Key Features
- External **32.768 kHz crystal** for RTC and low-power operation
- RC-controlled **EN** pin for reliable startup/reset behavior
- **SPI**, **I²S**, and **I²C** interfaces for peripheral integration

The ESP32 manages audio streaming, storage access, and system power-state control.

---

## 5. External NOR Flash Memory

An **SPI NOR flash** device provides non-volatile storage.

- SPI signals: **CS**, **SCLK**, **MOSI**, **MISO**
- Local decoupling minimizes supply impedance. Capacitor impedance is:

\[
Z_C = \frac{1}{j\omega C}
\]

Typical use cases include firmware assets and audio data storage.

---

## 6. Battery Charger and Power Management

A single-cell Li-ion battery charger IC handles charging and monitoring.

### Charging Control
- **Constant Current / Constant Voltage (CC/CV)** algorithm
- Charge current set by an external resistor:

\[
I_{CHG} = \frac{V_{REF}}{R_{PROG}}
\]

### Monitoring
- Battery voltage
- Charging state
- Temperature via **NTC**
- Communication over **I²C**

---

## 7. Digital-to-Analog Converter (DAC)

The DAC receives audio data via **I²S**.

Bit-clock relationship:

\[
f_{BCLK} = N_{bits} \cdot f_s
\]

Separate digital and analog supply domains reduce noise coupling and improve audio performance.

---

## 8. Audio Amplifier and Headphone Output (Detailed)

The audio output stage is implemented using a **stereo headphone amplifier (IC6)** that drives a **3-pole TRS jack (J2)**.  
The amplifier interfaces the DAC’s line-level outputs to **low-impedance headphone loads** while maintaining stability, low noise, and controlled frequency response.

### 8.1 Signal Path

- The DAC generates left/right analog outputs (e.g., **OUT_L**, **OUT_R**).
- These signals are routed to the amplifier input pins (left/right input nets).
- The amplifier outputs are routed to the headphone jack:

  - **HPLEFT → TIP**
  - **HPRIGHT → RING**
  - **SLEEVE → GND**

This is the standard stereo TRS wiring convention.

### 8.2 AC Coupling and DC Blocking

At the amplifier interface and/or output, capacitors are used to **block DC** and pass only the AC audio component. This prevents DC offsets from:

- shifting the headphone diaphragm operating point,
- generating audible clicks,
- increasing static current through the load.

If the headphone output is AC-coupled, the output capacitor \(C_{OUT}\) and the headphone impedance \(R_{LOAD}\) form a **first-order high-pass filter**:

\[
H(s)=\frac{s R_{LOAD} C_{OUT}}{1+s R_{LOAD} C_{OUT}}
\]

with cutoff frequency:

\[
f_c=\frac{1}{2\pi R_{LOAD} C_{OUT}}
\]

**Practical implication (example):** for typical headphones  
\(R_{LOAD}=32\,\Omega\), \(C_{OUT}=100\,\mu\text{F}\)

\[
f_c \approx \frac{1}{2\pi\cdot 32 \cdot 100\times 10^{-6}} \approx 49.7\,\text{Hz}
\]

A higher \(C_{OUT}\) reduces low-frequency attenuation (better bass response), at the cost of component size and possible inrush/pop behavior.

### 8.3 Input Filtering and Stability Considerations

Small RC networks around the amplifier input/output are commonly used to:

- limit RF pickup from digital activity (ESP32, I²S lines),
- improve amplifier stability with capacitive loads (cables/headphones),
- shape ultra-high-frequency response to reduce hiss and EMI.

A generic first-order low-pass (if present) is:

\[
f_{LP}=\frac{1}{2\pi R C}
\]

Even when not intended as an “audio” filter, such networks are important for **EMC robustness** and to prevent oscillations caused by headphone cable capacitance.

### 8.4 Power Supply Decoupling and Noise

A headphone amplifier can draw **dynamic current** correlated with the audio waveform; therefore supply impedance directly affects distortion and noise.  
Local decoupling capacitors provide a low AC impedance:

\[
Z_C=\frac{1}{j\omega C}
\]

Placing \(0.1\,\mu\text{F}\) (HF) + \(1\,\mu\text{F}\) (MF/LF) close to the amplifier supply pins reduces:

- audible ripple from the switching regulator,
- transient droop during bass peaks,
- pop/click during enable/disable.

### 8.5 Pop/Click Behavior (Startup/Shutdown)

AC coupling capacitors charge to a bias point during startup. The charging transient can produce an audible **pop** if not controlled.  
The pop magnitude is linked to the step component seen at the output:

\[
v_{pop}(t)\sim V_{step}\, e^{-t/(R_{eq}C)}
\]

where \(R_{eq}\) is the effective discharge/charge path. Practical mitigation includes:

- controlled enable sequencing (DAC before amplifier),
- adequate biasing/discharge paths,
- avoiding floating nodes at power transitions.

---

## 9. Grounding and Signal Integrity

- Analog (**AGND**) and digital (**DGND**) grounds are separated
- A single-point connection minimizes ground loops and common-impedance coupling
- Local decoupling capacitors (≈ **100 nF**) are placed at each IC supply pin

---

## 10. Design Philosophy

This schematic follows established **mixed-signal embedded design principles**:

- efficient power conversion,
- clear analog/digital domain separation,
- scalable peripheral interfaces,
- low-noise audio signal path.

The design is suitable for portable audio devices such as embedded music players or audio streaming systems.

