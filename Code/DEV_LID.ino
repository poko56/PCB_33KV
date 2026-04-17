#include <Arduino.h>

/* ======================================================================== 
   (ส่วนนี้คือลอจิกคณิตศาสตร์ไม่ต้องแก้)
   ======================================================================== */
class LeakageMonitor33kV {
private:
    uint8_t _pin;
    float _filteredVolts = 0.0f;
    float _display_uA = 0.0f;
    
    // ตั้งค่าความนิ่ง
    const float EMA_ALPHA = 0.02f;
    const float DEADBAND_UA = 3.0f;
    const float HYSTERESIS_WINDOW = 3.0f;
    const int OVERSAMPLING = 200;

    // ตารางสอบเทียบ 8 จุด
    const float rawV[8] = {0.0330, 0.0746, 0.1016, 0.1947, 0.2922, 0.3378, 0.3838, 0.5682};
    const float targetU[8] = {0.0, 6.6, 19.0, 59.2, 99.2, 118.6, 140.0, 216.0};

    float mapLUT(float vIn) {
        if (vIn <= rawV[0]) return targetU[0];
        if (vIn >= rawV[7]) {
            float slope = (targetU[7] - targetU[6]) / (rawV[7] - rawV[6]);
            return targetU[7] + (slope * (vIn - rawV[7]));
        }
        for (int i = 0; i < 7; ++i) {
            if (vIn >= rawV[i] && vIn <= rawV[i + 1]) {
                float slope = (targetU[i + 1] - targetU[i]) / (rawV[i + 1] - rawV[i]);
                return targetU[i] + (slope * (vIn - rawV[i]));
            }
        }
        return 0.0f;
    }

public:
    LeakageMonitor33kV(uint8_t pin) : _pin(pin) {}

    void begin() {
        analogReadResolution(12);
        _filteredVolts = (analogRead(_pin) * 3.3f) / 4095.0f; // Set initial state
    }

    float read_uA() {
        // 1. อ่านค่าและกรอง
        uint32_t sum = 0;
        for(int i = 0; i < OVERSAMPLING; ++i) { sum += analogRead(_pin); }
        float rawVolts = ((sum / (float)OVERSAMPLING) * 3.3f) / 4095.0f;
        _filteredVolts = (EMA_ALPHA * rawVolts) + ((1.0f - EMA_ALPHA) * _filteredVolts);

        // 2. คำนวณ uA
        float actual_uA = mapLUT(_filteredVolts);
        if (actual_uA < DEADBAND_UA) actual_uA = 0.0f;

        // 3. ลดการแกว่งของข้อมูล
        if (abs(actual_uA - _display_uA) > HYSTERESIS_WINDOW) {
            _display_uA = actual_uA;
        } else {
            _display_uA = (0.05f * actual_uA) + (0.95f * _display_uA);
        }
        return _display_uA;
    }
    
    // ฟังก์ชันเสริมเผื่ออยากดูค่าแรงดันดิบ
    float readVoltage() { return _filteredVolts; }
};

// 1. สร้างออบเจกต์เซนเซอร์ และระบุขาที่ใช้งาน (ใช้ขา 1)
LeakageMonitor33kV sensor(1); 

void setup() {
    Serial.begin(115200);
    
    // เริ่มต้นการทำงานของเซนเซอร์
    sensor.begin(); 
    
    Serial.println("System Ready! Waiting for data...");
}

void loop() {
    float current_uA = sensor.read_uA();
    Serial.printf("Leakage Current: %.1f uA\n", current_uA);
    
    // หน่วงเวลาการทำงานในแต่ละรอบ
    delay(500); 
}