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
    // Add signals for A0 (pin 36) - A 100Hz sine wave
    mock_signals_per_pin[36].push_back({SINE, 100.0f, 0.0f, 0.5f});
    mock_noise_per_pin[36] = 0.02f;

    // Add signals for A1 (pin 39) - 100Hz sine wave, 90 degree phase shift (PI/2 rad)
    mock_signals_per_pin[39].push_back({SINE, 100.0f, (float)PI / 2.0f, 0.5f});
    mock_noise_per_pin[39] = 0.02f;

    setup();
    for(int i=0; i<300; ++i) { // Run enough loops for at least one full buffer collection
        loop();
        current_micros += 1000;
    }
    return 0;
}
