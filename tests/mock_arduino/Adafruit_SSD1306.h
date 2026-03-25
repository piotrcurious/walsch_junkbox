#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Adafruit_GFX.h"
#include "Wire.h"
#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <cstdio>

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
      // Print the frame to stdout in a compact hex format for the test runner to capture
      std::cout << "DISPLAY_FRAME: ";
      for (int i = 0; i < (_width * _height + 7) / 8; ++i) {
          uint8_t b = 0;
          for (int bit = 0; bit < 8; ++bit) {
              int idx = i * 8 + bit;
              if (idx < _width * _height && buffer[idx]) {
                  b |= (1 << bit);
              }
          }
          std::printf("%02X", b);
      }
      std::cout << std::endl;

      // Save to a PBM for evaluation
      save_pbm("tests/output/last_frame.pbm");
  }

  void save_pbm(const char* filename) {
      mkdir("tests", 0777);
      mkdir("tests/output", 0777);
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
