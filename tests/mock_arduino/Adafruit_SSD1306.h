#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Adafruit_GFX.h"
#include "Wire.h"
#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>

#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0

class Adafruit_SSD1306 : public Adafruit_GFX {
public:
  Adafruit_SSD1306(int w, int h, TwoWire* w_ptr = &Wire, int rst = -1) : Adafruit_GFX(w, h) {
      buffer.assign(w * h, 0);
  }
  Adafruit_SSD1306(int sda, int scl, int rst) : Adafruit_GFX(128, 64) {
      buffer.assign(128 * 64, 0);
  }

  bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0x3C, bool reset = true, bool periphBegin = true) { return true; }

  void clearDisplay() {
      std::fill(buffer.begin(), buffer.end(), 0);
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
      if (x >= 0 && x < _width && y >= 0 && y < _height) {
          buffer[y * _width + x] = (color > 0 ? 1 : 0);
      }
  }

  void display() {
      // Create output directory if it doesn't exist
      mkdir("tests", 0777);
      mkdir("tests/output", 0777);

      static int frame_count = 0;
      std::string filename = "tests/output/display_output_" + std::to_string(frame_count++) + ".pbm";
      std::ofstream f(filename);
      if (f.is_open()) {
          f << "P1\n" << _width << " " << _height << "\n";
          for (int i = 0; i < _width * _height; ++i) {
              f << (int)buffer[i] << (i % _width == _width - 1 ? "\n" : " ");
          }
          f.close();
      }
  }

private:
  std::vector<uint8_t> buffer;
};

#endif
