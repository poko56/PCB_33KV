# ⚡ 33kV Leakage Current Monitor (ESP32-C3)

![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)
![Hardware](https://img.shields.io/badge/hardware-ESP32--C3--Super--Mini-orange.svg)
![Status](https://img.shields.io/badge/status-Production_Ready-success.svg)

High-Precision Leakage Current Monitor for 33kV Systems. This project utilizes an ESP32-C3 Super Mini paired with a custom TL072 Op-Amp circuit to detect and monitor microampere (µA) leakage currents from high-voltage transmission lines, providing ultra-stable readings using advanced DSP filtering.

**Created by:** hiw.makmakkub production by POKOMAN

---

## 🛠️ Hardware Design & Revisions

The hardware has been meticulously tuned for **50Hz AC signal integrity** and optimal ADC resolution. Below is the documentation of the custom PCB design.

### 1. Schematic Blueprint
![Schematic Blueprint](Schematic_PCB_33KV_2026-04-17.png)
* **Gain Tuning (2x):** Op-Amp U5.1 gain is set to 2x (`R11 = 10kΩ`, `R6 = 10kΩ`) to prevent waveform clipping from high-amplitude leakage spikes.
* **50Hz Signal Integrity:** Capacitor `C8 (1uF)` was **REMOVED** from the feedback loop to prevent the low-pass filter from attenuating the critical 50Hz AC leakage signal.
* **ADC Protection:** Added a **3.3V Zener Diode** in series with a 1kΩ resistor before the ESP32 `ADC_IN` pin (Pin 1) to clamp overvoltage and protect the microcontroller.

### 2. PCB Layout (2-Layer)
![PCB Fabrication Layout](PCB_PCB_PCB_33KV_2026-04-17.png)
* Strategic component placement isolating the analog front-end (TL072) from the digital processing unit (ESP32-C3) to minimize cross-talk and RF noise.
* Robust grounding plane for stable reference voltage.

### 3. 3D Render
![3D PCB Render](3D_PCB.png)
* Industrial-grade form factor with clear silkscreen references for easy field maintenance and calibration.

---

## 🧠 Software Architecture (OOP)

The firmware is written in modern C++ utilizing Object-Oriented Programming (OOP) principles for modularity, maintainability, and high-performance Digital Signal Processing (DSP).

### Core Features:
* **`SignalFilter` (EMA):** An Exponential Moving Average filter (`Alpha = 0.15`) that actively suppresses ADC noise while maintaining fast response times.
* **`HysteresisDisplay`:** A dynamic UI stabilization algorithm (`Window = 0.5 µA`) that prevents digit flickering on static loads.
* **`CalibratorLUT`:** An 8-point Lookup Table using Linear Interpolation and Extrapolation to precisely map raw voltage to true microampere (µA) leakage.
* **Oversampling Engine:** Samples the ADC 50 times per loop to exponentially increase virtual resolution.

---

## ⚙️ Calibration Data (LUT)
The system is calibrated using a precision standard multimeter. Current Lookup Table (LUT) maps ESP32-C3 12-bit ADC voltage to actual µA:

| Point | Filtered Voltage (V) | Actual Leakage (µA) |
| :---: | :---: | :---: |
| 1 | 0.0330 | 0.0 |
| 2 | 0.0746 | 6.6 |
| 3 | 0.1016 | 19.0 |
| 4 | 0.1947 | 59.2 |
| 5 | 0.2922 | 99.2 |
| 6 | 0.3378 | 118.6 |
| 7 | 0.3838 | 140.0 |
| 8 | 0.5682 | 216.0 |

> **Note:** If Op-Amp components (R11, R6) are modified, the LUT array in `33kV_Leakage_Monitor.cpp` must be recalibrated.

---

## 🚀 Getting Started

### Prerequisites
* Arduino IDE 2.x or PlatformIO
* ESP32 Board Package installed
* **Important:** Select `ESP32C3 Dev Module` and enable **USB CDC On Boot** in the Tools menu before flashing.

### Installation
1. Clone this repository.
2. Open the main `.ino` or `.cpp` file.
3. Verify `ADC_PIN` matches your hardware wiring (Default: `Pin 1`).
4. Compile and flash to the ESP32-C3 Super Mini.
5. Open Serial Monitor at `115200` baud rate to view real-time telemetry.

---
*Designed & Engineered for Industrial Reliability.*
