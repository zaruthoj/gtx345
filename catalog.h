#ifndef CATALOG_H
#define CATALOG_H

#include "listener.h"

#ifdef __x86_64__
#include <cstdlib>
#include "tests/u8g2_mock.h"
#else
#include <Arduino.h>
#include <U8g2lib.h>
#endif

#include <CircularBuffer.h>
#include <string.h>

#define TOP 12
#define BOTTOM 60
#define LEFT 0
#define RIGHT 240

#define MAX_FLIGHT_ID 8
#define MAX_EDIT_TEXT MAX_FLIGHT_ID

class Controller : public Listener {
 public:
  Controller(U8G2* screen) : screen_(screen) {}

  virtual void on_event(Event event_id, bool is_start=true);

  void process_event();
  void begin() {
    screen_->begin();
  }
  inline U8G2 * const get_screen() {
    return screen_; 
  }

  void set_flight_id(const char * flight_id) {
    strncpy(flight_id_, flight_id, MAX_FLIGHT_ID+1);
    flight_id_[MAX_FLIGHT_ID] = '\0';
    flight_id_len_ = strlen(flight_id_);
  }

  char squawk_code_[5] = "1200";
  char old_code_[5] = "";
  char flight_id_[MAX_FLIGHT_ID+1] = "N924DB";
  uint8_t flight_id_len_ = 6;
 private:
   struct EventData {
     Event event_id;
     bool is_start;
   };
   CircularBuffer<EventData, 5> event_queue_;
   U8G2 * const screen_;
};

class EditTile : public Tile {
 public:
  EditTile(uint8_t x, uint8_t y, uint8_t char_width, const uint8_t* font, uint8_t length) : 
      x_(x), y_(y), char_width_(char_width), font_(font), length_(length) {}

  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();
  virtual void clear_edit_text() {
    edit_text_[0] = '\0';
    edit_text_len_ = 0;
  }
  void append_edit_text(char new_char) {
    edit_text_[edit_text_len_] = new_char;
    if (edit_text_len_ < MAX_EDIT_TEXT) ++edit_text_len_;
    edit_text_[edit_text_len_] = '\0';
  }
  void backspace_edit_text() {
    if (edit_text_len_ > 0) --edit_text_len_;
    edit_text_[edit_text_len_] = '\0';
  }

  virtual char get_cursor_char() { return '_'; }

  char edit_text_[MAX_EDIT_TEXT+1] = "";
 protected:
  void exit_entry();
  virtual void apply_digit(char digit);
  virtual void clear_digit();
  virtual bool check_timeouts();

  uint8_t x_;
  uint8_t y_;
  uint8_t edit_text_len_ = 0;
  int8_t edit_digit_ = -1;
  uint8_t length_;

 private:
  uint8_t get_digit(Event event_id);
  virtual bool is_valid_digit(uint8_t digit);
  virtual bool maybe_set_value(Event event_id) = 0;
  virtual char * get_value() = 0;

  uint8_t char_width_;
  const uint8_t* font_;
  uint32_t clear_press_ms_ = 0;
};

class SquawkDisplay : public EditTile {
 public:
  SquawkDisplay() : EditTile(LEFT + 22, TOP + 41, 25, u8g2_font_inb30_mr, 4) {}
  virtual void on_change_event(Event event_id, bool is_start=true);

 private:
  virtual bool maybe_set_value(Event event_id);
  virtual char * get_value();
};

class StaticText : public Tile {
 public:
  StaticText(const char * text, uint8_t x, uint8_t y, const uint8_t* font) :
      font_(font),
      x_(x),
      y_(y),
      text_(text) {}

  virtual void render();
  virtual void on_change_event(Event event_id, bool is_start=true) {}
 private:  
  const uint8_t* font_;
  uint8_t x_;
  uint8_t y_;
  const char * text_;
};

class IdentText : public StaticText {
 public:
  IdentText(const char * text, uint8_t x, uint8_t y, const uint8_t* font) :
      StaticText(text, x, y, font) {}
  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();
 private:
  uint32_t off_time_ms_ = 0;
};

class StatusPage : public Tile {
 public:
  virtual void on_change_event(Event event_id, bool is_start=true);
};

#if 0
class FunctionPage: public Tile {
 public:
  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();
};
#endif

class FunctionGroup: public Tile {
 public:
  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();

  void add_function(Tile *function);
 private:
  CircularBuffer<Tile *, 4> functions_;
  int8_t current_function_ = 0;
};

class FlightId : public Tile {
 public:
  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();
};

class FlightIdEdit : public EditTile {
 public:
  FlightIdEdit() : EditTile(RIGHT - 1 - 14 * 8, TOP + 26 + 1, 14, u8g2_font_inb16_mr , 8) {}
  virtual void render();
  virtual char get_cursor_char();
 private:
  virtual bool maybe_set_value(Event event_id);
  virtual char * get_value();
  virtual bool is_valid_digit(uint8_t digit);
  virtual bool check_timeouts();
  virtual void apply_digit(char digit);
  virtual void clear_digit();

  uint32_t last_digit_ms_ = 0;
  int8_t ring_digit_ = -1;
  int8_t digit_group_ = -1;
};


#endif
