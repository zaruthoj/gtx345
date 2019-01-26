#ifndef U8G2_MOCK_H
#define U8G2_MOCK_H

#include <cstdlib>
#include <stdint.h>

#define PROGMEM

extern const uint8_t * u8g2_font_inb30_mr;
extern const uint8_t * u8g2_font_inb16_mr;
extern const uint8_t * u8g2_font_7x13B_tf;
extern const uint8_t * u8g2_font_7x13_mf;

extern uint32_t mock_time_ms;

extern void reset_mock_time();

extern void advance_mock_time(uint32_t ms);

extern uint32_t millis();

class U8G2 {
 public:

  virtual void begin() {}
  virtual void firstPage() {};
  virtual bool nextPage() { return false; }
  virtual void setDrawColor(uint8_t color) = 0;
  virtual void setFont(const uint8_t *font) = 0;
  virtual void drawStr(uint8_t x, uint8_t y, const char *str) = 0;
  virtual void drawGlyph(uint8_t x, uint8_t y, int16_t glyph) = 0;
  virtual void drawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3) {}
  virtual void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {}

  virtual ~U8G2() {}
};
#endif

