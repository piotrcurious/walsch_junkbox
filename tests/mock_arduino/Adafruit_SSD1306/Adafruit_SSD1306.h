#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Adafruit_GFX.h"
#include "Wire.h"

#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_WHITE 1

class Adafruit_SSD1306 : public Adafruit_GFX {
public:
  Adafruit_SSD1306(int w, int h, TwoWire* w_ptr = &Wire, int rst = -1) : Adafruit_GFX(w, h) {}
  // Special constructor used in some sketches
  Adafruit_SSD1306(int sda, int scl, int rst) : Adafruit_GFX(128, 64) {}

  bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0x3C, bool reset = true, bool periphBegin = true) { return true; }
  void clearDisplay() {}
  void display() {}
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {}
};

#endif
