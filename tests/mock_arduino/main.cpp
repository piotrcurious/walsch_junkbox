#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_SSD1306.h"
#include <vector>

Serial_ Serial;
TwoWire Wire;
std::vector<MockSignal> mock_signals;
uint32_t current_micros = 0;

int main() {
    // Add some default signals for testing
    mock_signals.push_back({100.0f, 0.0f, 0.5f}); // 100 Hz signal

    setup();
    for(int i=0; i<100; ++i) { // Run more loops for signal sampling
        loop();
        current_micros += 1000; // Progress time slightly each loop if not delayed
    }
    return 0;
}
