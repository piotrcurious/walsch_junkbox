#ifndef ARDUINO_H
#define ARDUINO_H

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <map>
#include <cstdlib>
#include <sstream>

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define HIGH 0x1
#define LOW 0x0

#define WHITE 1
#define BLACK 0

// ESP32 Analog Pins
#define A0 36
#define A1 39

typedef uint8_t byte;
typedef bool boolean;

inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return 0; }

enum WaveType { SINE, SQUARE, SAWTOOTH };

struct MockSignal {
    WaveType type;
    float freq;
    float phase;
    float amp;
    float offset;
};

extern std::map<uint8_t, std::vector<MockSignal>> mock_signals_per_pin;
extern std::map<uint8_t, float> mock_noise_per_pin;
extern uint32_t current_micros;

inline int analogRead(uint8_t pin) {
    float val = 0;
    float t = current_micros / 1000000.0f;
    if (mock_signals_per_pin.count(pin)) {
        for (auto& s : mock_signals_per_pin[pin]) {
            float arg = 2.0f * (float)PI * s.freq * t + s.phase;
            float sample = 0;
            switch(s.type) {
                case SINE:
                    sample = s.amp * std::sin(arg);
                    break;
                case SQUARE:
                    sample = s.amp * (std::sin(arg) >= 0 ? 1.0f : -1.0f);
                    break;
                case SAWTOOTH:
                    sample = s.amp * (2.0f * (arg / (2.0f * (float)PI) - std::floor(0.5f + arg / (2.0f * (float)PI))));
                    break;
            }
            val += sample + s.offset;
        }
    }
    if (mock_noise_per_pin.count(pin)) {
        val += mock_noise_per_pin[pin] * ((std::rand() % 2000 - 1000) / 1000.0f);
    }

    current_micros += 100; // Simulate ADC conversion time (~100us)
    int result = (int)((val + 1.0f) * 2047.5f);
    if (result < 0) result = 0;
    if (result > 4095) result = 4095;
    return result;
}

inline void analogReadResolution(int res) {}
#define ADC_11db 3
inline void analogSetAttenuation(int atten) {}

inline void delay(unsigned long ms) { current_micros += ms * 1000; }
inline void delayMicroseconds(unsigned int us) { current_micros += us; }
inline unsigned long millis() { return current_micros / 1000; }
inline unsigned long micros() { return current_micros; }

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline long random(long min, long max) {
  if (min == max) return min;
  return min + (std::rand() % (max - min));
}

inline long random(long max) {
  return random(0, max);
}

class Serial_ {
public:
  void begin(unsigned long baud) {}
  template<typename T> void print(T n) { std::cout << n; }
  template<typename T> void println(T n) { std::cout << n << std::endl; }
  void println() { std::cout << std::endl; }

  int available() {
    return (int)input_buffer.str().length() - input_ptr;
  }

  int read() {
    if (available()) {
        return input_buffer.str()[input_ptr++];
    }
    return -1;
  }

  void mock_input(const std::string& s) {
    input_buffer.str("");
    input_buffer.clear();
    input_buffer << s;
    input_ptr = 0;
  }

private:
  std::stringstream input_buffer;
  int input_ptr = 0;
};

extern Serial_ Serial;

#endif
