#ifndef ADAFRUIT_GFX_H
#define ADAFRUIT_GFX_H

#include <string>
#include <cstdint>

class Adafruit_GFX {
public:
  Adafruit_GFX(int w, int h) {}
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {}
  void setCursor(int16_t x, int16_t y) {}
  void setTextColor(uint16_t c) {}
  void setTextSize(uint8_t s) {}
  template<typename T> void print(T n) {}
  template<typename T> void println(T n) {}
  void println() {}
};

#endif
