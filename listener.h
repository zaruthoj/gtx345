#ifndef LISTENER_H
#define LISTENER_H

#ifdef __x86_64__
#include <cstdlib>
#else
#include <Arduino.h>
#endif

enum Event {
  BUTTON_CRSR = 0x00,
  BUTTON_FUNC,
  BUTTON_ENT,
  BUTTON_CLR,
  BUTTON_D9,
  BUTTON_D8,
  BUTTON_D5,
  BUTTON_D6,
  BUTTON_STBY = 0x10,
  BUTTON_ON,
  BUTTON_IDNT,
  BUTTON_ALT,
  BUTTON_VFR,
  BUTTON_OFF,
  BUTTON_D0,
  BUTTON_D1,
  BUTTON_D4 = 0x20,
  BUTTON_D3,
  BUTTON_D2,
  BUTTON_D7,
  EVENT_RENDER = 0xA0,
  EVENT_TICK = 0xA1,
};

class Listener {
 public:
  Listener();

  virtual void on_event(Event event_id, bool is_start=true);

  void add_child(Listener *child, bool active);
  
  void activate_child(Listener *child);
  void deactivate_child(Listener *child);
  void toggle_child(Listener *child);


  void unlink(Listener **head_ptr);
  void push(Listener **head_ptr);
  
  Listener* next_ = NULL;
  Listener* prev_ = NULL;
  Listener* active_ = NULL;
  Listener* standby_ = NULL;

 protected:
  Listener* parent_ = NULL;
  
 private:
};

class Tile : public Listener {
 public:
  virtual void on_event(Event event_id, bool is_start=true);
  virtual void render() {}
  virtual void on_change_event(Event event_id, bool is_start=true) {}
};

#endif
