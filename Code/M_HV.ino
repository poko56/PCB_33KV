/**
 * -----------------------------------------------------------------------------
 * @file    33kV_Leakage_Monitor.cpp
 * @brief   Firmware for ESP32-C3 Leakage Current Monitoring (33kV System)
 * @author  Supachok Hornsombat
 * @note    Hardware Revision: Op-Amp Gain = 2x (R11=10k, C8 Removed)
 * -----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <WiFi.h>
#include "esp_bt.h"

namespace Config {
    constexpr uint8_t  SENSOR_PIN         = 1;
    constexpr uint32_t SERIAL_BAUD        = 115200;

    // [TUNE] DSP Parameters 
    constexpr float    EMA_ALPHA          = 0.02f;  // IIR filter coeff (lower = smoother but higher latency)
    constexpr float    DEADBAND_UA        = 3.0f;   // Mute ADC noise floor at 0-load
    constexpr uint16_t OVERSAMPLING_COUNT = 200;    // Trade CPU cycles for virtual resolution
    constexpr float    HYSTERESIS_WINDOW  = 3.0f;   // Display lock window to prevent UI flickering

    constexpr uint32_t PRINT_INTERVAL_MS  = 500;    // Telemetry polling rate (2Hz)
}

/**
 * @class SignalFilter
 * @brief Exponential Moving Average (EMA) Low-pass filter
 */
class SignalFilter {
private:
    float _filteredValue;
    float _alpha;

public:
    SignalFilter(float alpha) : _filteredValue(0.0f), _alpha(alpha) {}

    void setInitialValue(float val) {
        _filteredValue = val;
    }

    float update(float rawValue) {
        _filteredValue = (_alpha * rawValue) + ((1.0f - _alpha) * _filteredValue);
        return _filteredValue;
    }

    float get() const { return _filteredValue; }
};

/**
 * @class HysteresisDisplay
 * @brief Stabilizes display output to prevent flickering values
 */
class HysteresisDisplay {
private:
    float _displayValue;
    float _window;

public:
    HysteresisDisplay(float window) : _displayValue(0.0f), _window(window) {}

    float update(float actualValue) {
        // Fast-tracking for large steps, smooth merging for micro-fluctuations
        if (abs(actualValue - _displayValue) > _window) {
            _displayValue = actualValue;
        } else {
            _displayValue = (0.05f * actualValue) + (0.95f * _displayValue);
        }
        return _displayValue;
    }
};

/**
 * @class CalibratorLUT
 * @brief Linear Interpolation for multi-point calibration data
 */
class CalibratorLUT {
private:
    static constexpr uint8_t NUM_POINTS = 8;
    
    // [WARN] LUT tied to specific hardware state. Recalibrate if op-amp gain changes.
    const float _rawVoltage[NUM_POINTS] = { 0.0330f, 0.0746f, 0.1016f, 0.1947f, 0.2922f, 0.3378f, 0.3838f, 0.5682f };
    const float _target_uA[NUM_POINTS]  = { 0.0f,    6.6f,    19.0f,   59.2f,   99.2f,   118.6f,  140.0f,  216.0f };

public:
    float convert(float vIn) const {
        // Under-range protection
        if (vIn <= _rawVoltage[0]) return _target_uA[0];
        
        // Extrapolation for over-range inputs
        if (vIn >= _rawVoltage[NUM_POINTS - 1]) {
            float slope = (_target_uA[NUM_POINTS - 1] - _target_uA[NUM_POINTS - 2]) / 
                          (_rawVoltage[NUM_POINTS - 1] - _rawVoltage[NUM_POINTS - 2]);
            return _target_uA[NUM_POINTS - 1] + (slope * (vIn - _rawVoltage[NUM_POINTS - 1]));
        }

        // Standard linear interpolation
        for (uint8_t i = 0; i < NUM_POINTS - 1; ++i) {
            if (vIn >= _rawVoltage[i] && vIn <= _rawVoltage[i + 1]) {
                float slope = (_target_uA[i + 1] - _target_uA[i]) / (_rawVoltage[i + 1] - _rawVoltage[i]);
                return _target_uA[i] + (slope * (vIn - _rawVoltage[i]));
            }
        }
        return 0.0f;
    }
};


// --- Global Instances ---
SignalFilter      adcFilter(Config::EMA_ALPHA);
HysteresisDisplay displayStabilizer(Config::HYSTERESIS_WINDOW);
CalibratorLUT     calibrator;

uint32_t previousSerialMillis = 0;


void disableWirelessRadios() {
    // Kill radios to save power and reduce RF noise coupling into the ADC
    btStop(); 
    WiFi.disconnect(true);  
    WiFi.mode(WIFI_OFF);
}

float readSensorOversampled() {
    uint32_t sum = 0;
    for(uint16_t i = 0; i < Config::OVERSAMPLING_COUNT; ++i) { 
        sum += analogRead(Config::SENSOR_PIN); 
    }
    return ((sum / (float)Config::OVERSAMPLING_COUNT) * 3.3f) / 4095.0f;
}

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    
    disableWirelessRadios();
    analogReadResolution(12);

    // Pre-seed the filter to prevent ramp-up latency on boot
    adcFilter.setInitialValue((analogRead(Config::SENSOR_PIN) * 3.3f) / 4095.0f);

    Serial.println("\n[SYSTEM BOOT] 33kV Leakage Monitor initialized");
}

void loop() {
    uint32_t currentMillis = millis();

    // Data Acquisition Pipeline
    float rawVolts = readSensorOversampled();
    float filteredVolts = adcFilter.update(rawVolts);

    // Feature Extraction
    float actualLeakage_uA = calibrator.convert(filteredVolts);

    // Noise Rejection
    if (actualLeakage_uA < Config::DEADBAND_UA) {
        actualLeakage_uA = 0.0f;
    }

    // UI Stabilization
    float displayLeakage_uA = displayStabilizer.update(actualLeakage_uA);

    // Asynchronous Telemetry Stream
    if (currentMillis - previousSerialMillis >= Config::PRINT_INTERVAL_MS) {
        previousSerialMillis = currentMillis;
        
        Serial.printf("V_Filter: %4.4f V | Actual: %6.1f uA | Display: %6.1f uA\n", 
                      filteredVolts, actualLeakage_uA, displayLeakage_uA);
    }
}