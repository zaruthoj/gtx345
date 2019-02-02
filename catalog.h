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
#define RIGHT 254

#define ALT_X LEFT
#define ALT_Y (TOP + 22)
#define IDNT_X LEFT
#define IDNT_Y (TOP + 8)

#define FLIGHT_ID_LABEL_CHAR_WIDTH 6
#define FLIGHT_ID_CHAR_WIDTH 14
#define FLIGHT_ID_CENTER_X 180
#define FLIGHT_ID_LABEL_X (FLIGHT_ID_CENTER_X - (FLIGHT_ID_LABEL_CHAR_WIDTH * 4))
#define FLIGHT_ID_LABEL_Y 20
#define FLIGHT_ID_Y 45

#define FLIGHT_ID_EDIT_LABEL_X (LEFT + 1)
#define FLIGHT_ID_EDIT_LABEL_Y (TOP + 20)
#define FLIGHT_ID_EDIT_LEGEND_LETTERS_Y BOTTOM
#define FLIGHT_ID_EDIT_LEGEND_NUMBERS_Y (BOTTOM - 12)
#define FLIGHT_ID_EDIT_LEGEND_CHAR_W 6

#define FLIGHT_ID_EDIT_CHAR_W 14
#define FLIGHT_ID_EDIT_X (RIGHT - 1 - FLIGHT_ID_EDIT_CHAR_W * 8)
#define FLIGHT_ID_EDIT_Y (TOP + 20)

#define SQUAWK_X (LEFT + 30)
#define SQUAWK_Y (TOP + 26)
#define SQUAWK_CHAR_W 25

#define FLIGHT_ID_LABEL_FONT u8g2_font_7x13B_tr
#define IDNT_FONT u8g2_font_7x13B_tr
#define ALT_FONT u8g2_font_7x13B_tr
#define FLIGHT_ID_EDIT_LEGEND_FONT u8g2_font_7x13B_tr

#define FLIGHT_ID_EDIT_LABEL_FONT u8g2_font_inb16_mr
#define FLIGHT_ID_FONT u8g2_font_inb16_mr
#define FLIGHT_ID_EDIT_FONT u8g2_font_inb16_mr

#define SQUAWK_FONT u8g2_font_inb30_mr

#define MAX_FLIGHT_ID 8
#define MAX_EDIT_TEXT MAX_FLIGHT_ID

class Controller : public Tile {
 public:
  Controller(U8G2* screen) : screen_(screen) {}

  virtual void on_event(Event event_id, bool is_start=true);
  virtual void on_change_event(Event event_id, bool is_start=true);
  virtual void render();
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

  bool power_on_ = true;
  bool alt_on_ = true;
  bool standby_ = false;
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

class SerialCom : public Listener {
 public:
  virtual void on_event(Event event_id, bool is_start=true);

 private:
  uint32_t last_watchdog_ms_ = 0;
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
  SquawkDisplay() : EditTile(SQUAWK_X, SQUAWK_Y, SQUAWK_CHAR_W, SQUAWK_FONT, 4) {}
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
  FlightIdEdit() : EditTile(FLIGHT_ID_EDIT_X, FLIGHT_ID_EDIT_Y, FLIGHT_ID_EDIT_CHAR_W, FLIGHT_ID_EDIT_FONT , 8) {}
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
