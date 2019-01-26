#include "catalog.h"
#include "listener.h"

extern Controller &controller(bool reset=false);
extern StaticText &alt_mode();


void Controller::on_event(Event event_id, bool is_start) {
  event_queue_.unshift(EventData{event_id, is_start});
}

void Controller::process_event() {
  if (event_queue_.isEmpty()) return;
  EventData event = event_queue_.last();
  if (event.event_id == EVENT_RENDER) {
    screen_->firstPage();
    do {
      Listener::on_event(EVENT_RENDER, event.is_start);
    } while ( screen_->nextPage() );
  } else {
    Listener::on_event(event.event_id, event.is_start);
  }
  event_queue_.pop();
}

uint8_t EditTile::get_digit(Event event_id) {
  switch(event_id) {
    case BUTTON_D0:
      return 0;
    case BUTTON_D1:
      return 1;
    case BUTTON_D2:
      return 2;
    case BUTTON_D3:
      return 3;
    case BUTTON_D4:
      return 4;
    case BUTTON_D5:
      return 5;
    case BUTTON_D6:
      return 6;
    case BUTTON_D7:
      return 7;
    case BUTTON_D8:
      return 8;
    case BUTTON_D9:
      return 9;
    default:
      return 255;
  }
}

bool EditTile::is_valid_digit(uint8_t digit) {
  return digit <= 7;
}

void EditTile::exit_entry() {
  edit_digit_ = -1;
  clear_edit_text();
}

void EditTile::apply_digit(char digit) {
  append_edit_text(digit);
  if (edit_digit_ < 0) edit_digit_ = 0;
  ++edit_digit_;
}

void EditTile::clear_digit() {
  edit_digit_--;
  backspace_edit_text();
}

bool EditTile::check_timeouts() {
  if (clear_press_ms_ > 0 && millis() > clear_press_ms_ + 500) {
    edit_digit_ = 0;
    clear_edit_text();
    clear_press_ms_ = 0;
    return true;
  }
  return false;
}

void EditTile::on_change_event(Event event_id, bool is_start) {
  bool should_render = false;
  
  if (is_start) {
    uint8_t digit = get_digit(event_id);
    if (is_valid_digit(digit) && edit_digit_ < length_) {
      apply_digit('0' + digit);
      should_render = true;
    }
    
    if (edit_digit_ >= 0) {
      switch(event_id) {
        case BUTTON_CRSR:
          exit_entry();
          should_render = true;
          break;
        case BUTTON_CLR:
          clear_press_ms_ = millis();
          break;
        case EVENT_TICK:
          should_render |= check_timeouts();
          break;
      }
      if (maybe_set_value(event_id)) {
        exit_entry();
        should_render = true;
      }
    }
  } else if (event_id == BUTTON_CLR) {
    if (clear_press_ms_ > 0) {
      clear_press_ms_ = 0;
      if (edit_digit_ > 0) {
        clear_digit();
        should_render = true;
      } else if (edit_digit_ == 0) {
        exit_entry();
        should_render = true;
      }
    }
  }

  if (should_render) {
    controller().on_event(EVENT_RENDER);
  }
}

void EditTile::render() {
  controller().get_screen()->setDrawColor(1);
  controller().get_screen()->setFont(font_);
  if (edit_digit_ == -1) {
    controller().get_screen()->drawStr(x_, y_, get_value());
  } else {
    controller().get_screen()->setDrawColor(1);
    controller().get_screen()->setFont(font_);
    controller().get_screen()->drawStr(x_, y_, edit_text_);
    for (int i = edit_digit_ + 1; i < length_; ++i) {
      controller().get_screen()->drawGlyph(x_ + char_width_ * (edit_digit_ + i + 1), y_, '_');
    }
    controller().get_screen()->setDrawColor(0);
    controller().get_screen()->drawGlyph(x_ + char_width_ * edit_digit_, y_, get_cursor_char());
  }
}

void SquawkDisplay::on_change_event(Event event_id, bool is_start) {
  if (is_start && event_id == BUTTON_VFR) {
    if (controller().old_code_[0] != '\0' && strcmp(controller().squawk_code_, "1200") == 0) {
      strncpy(controller().squawk_code_, controller().old_code_, 5);
      controller().old_code_[0] = '\0';
    } else {
      strncpy(controller().old_code_, controller().squawk_code_, 5);
      strncpy(controller().squawk_code_, "1200", 5);
    }
    exit_entry();
    controller().on_event(EVENT_RENDER);
  }
  EditTile::on_change_event(event_id, is_start);
}

bool SquawkDisplay::maybe_set_value(Event event_id) {
  if (edit_digit_ >= length_) {
    strncpy(controller().squawk_code_, edit_text_, 5);
    return true;
  }
  return false;
}

char * SquawkDisplay::get_value() {
  return controller().squawk_code_;
}

void StaticText::render() {
  controller().get_screen()->setDrawColor(1);
  controller().get_screen()->setFont(font_);
  controller().get_screen()->drawStr(x_, y_, text_);
}

void StatusPage::on_change_event(Event event_id, bool is_start) {
  if (event_id == BUTTON_ALT && is_start) {
    toggle_child(&alt_mode());
    controller().on_event(EVENT_RENDER);
  }
}

void IdentText::on_change_event(Event event_id, bool is_start) {
  if (event_id == BUTTON_IDNT && is_start) {
    off_time_ms_ = millis() + 18 * 1000;
    controller().on_event(EVENT_RENDER);
  }
  if (event_id == EVENT_TICK) {
    if (off_time_ms_ > 0 && millis() > off_time_ms_) {
      off_time_ms_ = 0;
      controller().on_event(EVENT_RENDER);
    }
  }
}

void IdentText::render() {
  if (off_time_ms_ > 0) StaticText::render();
}

void FunctionGroup::on_change_event(Event event_id, bool is_start) {
  if (is_start) {
    switch (event_id) {
      case BUTTON_D8:
        deactivate_child(functions_[current_function_]);
        --current_function_;
        if (current_function_ < 0) current_function_ = functions_.size() - 1;
        activate_child(functions_[current_function_]);
        controller().on_event(EVENT_RENDER);
        break;
      case BUTTON_D9:
        deactivate_child(functions_[current_function_]);
        current_function_ = (current_function_ + 1) % functions_.size();
        activate_child(functions_[current_function_]);
        controller().on_event(EVENT_RENDER);
        break;
    }
  }
}

void FunctionGroup::render() {
  const PROGMEM uint8_t x = 230;
  controller().get_screen()->setDrawColor(1);
  controller().get_screen()->drawTriangle(x, 18, x+4, 13, x+8, 18);
  controller().get_screen()->drawTriangle(x, 59, x+8, 59, x+4, 64);
  uint8_t h = 39 / functions_.size();
  controller().get_screen()->drawBox(x+2, 19 + h * current_function_, 5, h);
}

void FunctionGroup::add_function(Tile *function) {
  functions_.push(function);
  add_child(function, functions_.size() == 1);
}

void FlightId::on_change_event(Event event_id, bool is_start) {

}

void FlightId::render() {
  controller().get_screen()->setDrawColor(1);
  controller().get_screen()->setFont(u8g2_font_7x13B_tf);
  controller().get_screen()->drawStr(146, 30, "FLIGHT ID");
  controller().get_screen()->setFont(u8g2_font_inb16_mr);
  controller().get_screen()->drawStr(173 - controller().flight_id_len_ * 7 , 50, controller().flight_id_);
}

bool FlightIdEdit::maybe_set_value(Event event_id) {
  if (event_id == BUTTON_ENT) {
    if(digit_group_ != -1) {
      EditTile::apply_digit(get_cursor_char());
      ring_digit_ = -1;
      digit_group_ = -1;
    }
    controller().set_flight_id(edit_text_);
    return true;
  }
  return false;
}

char * FlightIdEdit::get_value() {
  return controller().flight_id_;
}

bool FlightIdEdit::is_valid_digit(uint8_t digit) {
  return digit <= 9;
}

void FlightIdEdit::apply_digit(char digit) {
  uint8_t ring_size = 4;
  digit = digit - '0';
  if (digit == 8) ring_size = 3;

  if (edit_digit_ < 0) edit_digit_ = 0;
  if (digit_group_ != -1 && digit_group_ != digit) {
    EditTile::apply_digit(get_cursor_char());
    ring_digit_ = -1;
  }

  if (digit == 9) {
    EditTile::apply_digit(digit + '0');
    ring_digit_ = -1;
    digit_group_ = -1;
    last_digit_ms_ = -1;
    return;
  }

  digit_group_ = digit;
  ring_digit_ = (ring_digit_ + 1) % ring_size;
  last_digit_ms_ = millis();
}

char FlightIdEdit::get_cursor_char() {
  if (digit_group_ == -1) return '_';
  if (ring_digit_ == 0) return '0' + digit_group_;
  return 'A' + digit_group_ * 3 + ring_digit_ - 1;
}

void FlightIdEdit::clear_digit() {
  if (digit_group_ != -1) {
    ring_digit_ = -1;
    digit_group_ = -1;
    last_digit_ms_ = -1;
  } else {
    EditTile::clear_digit();
  }
}

bool FlightIdEdit::check_timeouts() {
  bool should_render = false;
  if (last_digit_ms_ > 0 && millis() > last_digit_ms_ + 2000 && digit_group_ != -1) {
    EditTile::apply_digit(get_cursor_char());
    ring_digit_ = -1;
    digit_group_ = -1;
    last_digit_ms_ = -1;
    should_render = true;
  }
  return EditTile::check_timeouts() | should_render;
}

void FlightIdEdit::render() {
  controller().get_screen()->setDrawColor(1);
  controller().get_screen()->setFont(u8g2_font_7x13B_tf);
  controller().get_screen()->drawStr(LEFT + 1, TOP + 13 + 2, "FLIGHT ID");
  controller().get_screen()->setFont(u8g2_font_7x13_mf);
  for (int i = 0; i < 10; ++i) {
    uint8_t x = LEFT + 1 + i * (RIGHT - LEFT - 2 - 12) / 9;
    controller().get_screen()->drawGlyph(x + 6, BOTTOM + 1, '0' + i);
    uint8_t cx = x;
    if (i == 8) cx += 3;
    for (int j = 0; j < 3 && i * 3 + j < 26; ++j) {
      controller().get_screen()->drawGlyph(cx + j*6, BOTTOM + 1 + 7, 'A' + i * 3 + j);
    }
  }
  
  EditTile::render();
}
