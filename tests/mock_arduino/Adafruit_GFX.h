#ifndef ADAFRUIT_GFX_H
#define ADAFRUIT_GFX_H

#include <string>
#include <cstdint>
#include <iostream>

class Adafruit_GFX {
public:
  Adafruit_GFX(int w, int h) : _width(w), _height(h) {}
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) {
      if(x < 0 || x >= _width || y < 0 || y >= _height) {
          // std::cerr << "OOB drawing: (" << x << ", " << y << ")" << std::endl;
      }
  }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {}
  void setCursor(int16_t x, int16_t y) {}
  void setTextColor(uint16_t c) {}
  void setTextSize(uint8_t s) {}
  template<typename T> void print(T n) {}
  template<typename T> void println(T n) {}
  void println() {}
protected:
  int _width, _height;
};

#endif
