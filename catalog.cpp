#include <cstddef>
#include "catalog.h"
#include "listener.h"

extern Controller controller;
extern StaticText alt_mode;

void Controller::on_event(Event event_id, bool is_start=true) {
  if (event_id == EVENT_RENDER) {
    screen_->firstPage();
    do {
      Listener::on_event(EVENT_RENDER, is_start);
    } while ( screen_->nextPage() );
    return;
  }

  Serial.print(event_id);
  Serial.print(", ");
  Serial.println(is_start);
  Listener::on_event(event_id, is_start);
}

void SquawkDisplay::render() {
  controller.get_screen()->setFont(u8g2_font_ncenB18_tr);
  controller.get_screen()->drawStr(30,10, String(controller.squawk_code_).c_str());
}

void StaticText::render() {
  controller.get_screen()->setFont(font_);
  controller.get_screen()->drawStr(x_, y_, text_.c_str());
}

void StatusPage::on_change_event(Event event_id, bool is_start=true) {
  if (event_id == BUTTON_ALT && is_start) {
    Serial.println("ALT");
    toggle_child(&alt_mode);
  }
}
