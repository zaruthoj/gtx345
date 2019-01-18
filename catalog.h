#ifndef CATALOG_H
#define CATALOG_H

#include "listener.h"

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

class Controller : public Listener {
 public:
   Controller(U8G2* screen) : screen_(screen) {}

   virtual void on_event(Event event_id, bool is_start=true);

   void begin() {
     screen_->begin();
   }
   inline U8G2 * const get_screen() {
     return screen_; 
   }

  uint16_t squawk_code_ = 1200;
 private:
   U8G2 * const screen_;
};

class SquawkDisplay : public Tile {
 public:
  virtual void render();
};

class StaticText : public Tile {
 public:
  StaticText(String text, uint8_t x, uint8_t y, const uint8_t* font) :
      font_(font),
      x_(x),
      y_(y),
      text_(text) {}

 private:
  virtual void render();

  const uint8_t* font_;
  uint8_t x_;
  uint8_t y_;
  const String text_;
};

class StatusPage : public Tile {
 protected:
  virtual void on_change_event(Event event_id, bool is_start=true);
};

#endif
