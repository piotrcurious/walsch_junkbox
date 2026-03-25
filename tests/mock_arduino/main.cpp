#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_SSD1306.h"
#include <vector>
#include <map>

Serial_ Serial;
TwoWire Wire;
std::map<uint8_t, std::vector<MockSignal>> mock_signals_per_pin;
std::map<uint8_t, float> mock_noise_per_pin;
uint32_t current_micros = 0;

int main() {
    // Add signals for A0 (pin 36) - A 100Hz square wave
    mock_signals_per_pin[36].push_back({SQUARE, 100.0f, 0.0f, 0.5f});
    mock_noise_per_pin[36] = 0.01f;

    // Add signals for A1 (pin 39) - 100Hz square wave, 1.0 rad phase shift
    mock_signals_per_pin[39].push_back({SQUARE, 100.0f, 1.0f, 0.5f});
    mock_noise_per_pin[39] = 0.01f;

    setup();
    // Run for multiple collection periods to ensure display updates
    for(int i=0; i<500; ++i) {
        loop();
        current_micros += 1000;
    }
    return 0;
}
