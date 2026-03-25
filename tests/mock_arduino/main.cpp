#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_SSD1306.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstdio>

Serial_ Serial;
TwoWire Wire;
std::map<uint8_t, std::vector<MockSignal>> mock_signals_per_pin;
std::map<uint8_t, float> mock_noise_per_pin;
uint32_t current_micros = 0;

void process_mock_commands() {
    std::string line;
    if (std::getline(std::cin, line)) {
        if (line.find("SIG:") == 0) {
            int pin, type;
            float freq, phase, amp, offset;
            if (std::sscanf(line.c_str(), "SIG:%d:%d:%f:%f:%f:%f", &pin, &type, &freq, &phase, &amp, &offset) == 6) {
                mock_signals_per_pin[pin].clear();
                mock_signals_per_pin[pin].push_back({(WaveType)type, freq, phase, amp, offset});
            }
        } else if (line.find("NOISE:") == 0) {
            int pin;
            float val;
            if (std::sscanf(line.c_str(), "NOISE:%d:%f", &pin, &val) == 2) {
                mock_noise_per_pin[pin] = val;
            }
        }
    }
}

extern void setup();
extern void loop();

int main() {
    mock_signals_per_pin[36].push_back({SINE, 100.0f, 0.0f, 0.5f, 0.0f});
    mock_signals_per_pin[39].push_back({SINE, 100.0f, (float)PI / 2.0f, 0.5f, 0.0f});

    setup();
    while(true) {
        process_mock_commands();
        loop();
        current_micros += 1000;

        static int total_loops = 0;
        if (++total_loops > 10000) break;
    }
    return 0;
}
