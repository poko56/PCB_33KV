# ⚡ 33kV Leakage Current Monitor System

![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--C3-green.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-cyan.svg)
![Architecture](https://img.shields.io/badge/architecture-OOP-orange.svg)

ระบบตรวจวัดกระแสไฟรั่ว (Leakage Current) ความแม่นยำสูงสำหรับระบบไฟฟ้าแรงสูง 33kV ออกแบบมาเพื่อการทำงานที่เสถียรในสภาพแวดล้อมที่มีสัญญาณรบกวน (Noise) สูง ผสมผสานการปรับแต่งทางฮาร์ดแวร์ (Hardware Tuning) และการประมวลผลสัญญาณดิจิทัล (Digital Signal Processing - DSP) ขั้นสูง

## 🌟 Key Features

- **Multi-Stage DSP Pipeline:**
  - **Oversampling:** ดึงค่า 200 รอบต่อลูปเพื่อเพิ่มความละเอียดเสมือน (Virtual Resolution) และลด White Noise
  - **EMA Filter (Exponential Moving Average):** กรองสัญญาณความถี่สูงทิ้ง ทำให้ค่าแรงดัน (V) นิ่งสนิท
  - **Hysteresis UI Stabilizer:** ระบบแช่แข็งตัวเลขหน้าจอแบบไดนามิก ป้องกันตัวเลขกระพริบ (Flickering) ในระดับ Micro-fluctuations
- **8-Point LUT Calibration:** แปลงค่าแรงดันเป็นกระแสไมโครแอมป์ (µA) ด้วยระบบเทียบบัญญัติไตรยางศ์ (Linear Interpolation) 8 ระดับ แก้ปัญหากราฟไม่เป็นเส้นตรง (Non-linear response)
- **Hardware-Tuned:** ปรับจูนอัตราขยายของ Op-Amp เพื่อแก้ปัญหาสัญญาณขลิบ (Signal Clipping) และการลดทอนความถี่ 50Hz

## 🛠️ Hardware Modifications & Schematic Notes

เพื่อให้ซอฟต์แวร์ทำงานได้ประสิทธิภาพสูงสุด ต้องมีการปรับแต่งบอร์ดวงจรเซนเซอร์ดังนี้:

1. **Op-Amp Gain Adjustment (U5.1 - TL072):**
   - เปลี่ยนตัวต้านทาน **R11 เป็น 10kΩ** (จากเดิม 100kΩ) เพื่อลดอัตราขยายลงเหลือ **2 เท่า** (Gain = 2x) ป้องกันปัญหาสัญญาณยอดคลื่นล้น (Clipping)
2. **ESP32 Overvoltage Protection:**
   - ติดตั้ง **Zener Diode 3.3V** คั่นระหว่างวงจร Op-Amp และขา `ADC_IN` (Pin 1) ของ ESP32-C3 เพื่อป้องกันแรงดันกระชากทำลายชิป

## 💻 Software Architecture

ระบบทำงานผ่าน 3 คลาสหลัก (Core Classes):
1. `SignalFilter`: จัดการเรื่องความหน่วงและการกรอง Noise ของฮาร์ดแวร์
2. `CalibratorLUT`: แปลงค่า ADC Voltage เป็นค่ากระแส µA ผ่านตารางสอบเทียบ
3. `HysteresisDisplay`: ตัดสินใจและเกลี่ยค่าตัวเลขก่อนแสดงผล เพื่อให้ UI สมูทที่สุด

## 🚀 Quick Start / Setup

1. เชื่อมต่อสายสัญญาณ `ADC_IN` เข้ากับ **Pin 1** ของ ESP32-C3 Super Mini
2. เปิดไฟล์ใน Arduino IDE และตั้งค่าบอร์ดเป็น ESP32-C3
3. ไปที่เมนู `Tools` และตั้งค่า **USB CDC On Boot: Enabled** (สำคัญมากสำหรับการดู Serial Monitor บนชิป C3)
4. กด Upload โค้ด
5. เปิด Serial Monitor ที่ Baud Rate `115200`

## 📐 Calibration Guide (การสอบเทียบ)

หากมีการเปลี่ยนบอร์ด ESP32 หรือไอซี Op-Amp จะต้องทำการอัปเดตตาราง LUT ใหม่ โดยเข้าไปแก้ในคลาส `CalibratorLUT`:

```cpp
// [X] ค่าแรงดัน FilterV ที่บอร์ดอ่านได้ ณ ขณะนั้น
const float _rawVoltage[8] = { 0.0330f, 0.0746f, ... };

// [Y] ค่ากระแสไฟรั่วจริง (µA) ที่อ่านได้จากมิเตอร์มาตรฐาน
const float _target_uA[8]  = { 0.0f,    6.6f,   ... };
