#include "listener.h" 

Listener::Listener() {
}

void Listener::on_event(Event event_id, bool is_start) {
  Listener *child = active_;
  while (child != NULL) {
    child->on_event(event_id, is_start);
    child = child->next_;
  }
}

void Listener::add_child(Listener *child, bool active) {
  child->push(active ? &active_ : &standby_);
  child->parent_ = this;
}

void Listener::unlink(Listener **head_ptr) {
  if(*head_ptr == this) {
    *head_ptr = next_;
  }
  if (prev_) {
    prev_->next_ = next_;
  }
  if (next_) {
    next_->prev_ = prev_;
  }
  next_ = NULL;
  prev_ = NULL;
}

void Listener::push(Listener **head_ptr) {
  Listener *head = *head_ptr;
  if (head) head->prev_ = this;
  this->next_ = head;
  this->prev_ = NULL;
  *head_ptr = this;
}

void Listener::activate_child(Listener *child) {
  child->unlink(&standby_);
  child->push(&active_);
}

void Listener::deactivate_child(Listener *child) {
  child->unlink(&active_);
  child->push(&standby_);
}

void Listener::toggle_child(Listener *child) {
  Listener *test = child;
  while (test->prev_ != NULL) test = test->prev_;
  Listener **new_head = (test == active_) ? &standby_ : &active_;
  Listener **old_head = (test == active_) ? &active_ : &standby_;

  child->unlink(old_head);
  child->push(new_head);
}

void Tile::on_event(Event event_id, bool is_start) {
  if (event_id == EVENT_RENDER) {
    render();
  } else {
    on_change_event(event_id, is_start);
  }
  Listener::on_event(event_id, is_start);
}

