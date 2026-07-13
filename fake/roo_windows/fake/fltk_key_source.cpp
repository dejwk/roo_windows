#include "roo_windows/fake/fltk_key_source.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>

namespace roo_windows::fake {

FltkKeySource* FltkKeySource::active_source_ = nullptr;
bool FltkKeySource::handler_installed_ = false;

FltkKeySource::FltkKeySource() : head_(0), tail_(0) {
  active_source_ = this;
  if (!handler_installed_) {
    Fl::add_handler(handleFltkEvent);
    handler_installed_ = true;
  }
}

FltkKeySource::~FltkKeySource() {
  if (active_source_ == this) active_source_ = nullptr;
}

int FltkKeySource::drain(KeyEvent* out, int max_events) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  int count = 0;
  while (head_ != tail_ && count < max_events) {
    out[count++] = queue_[head_];
    head_ = (head_ + 1) % kQueueCapacity;
  }
  return count;
}

bool FltkKeySource::enqueue(const KeyEvent& event) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  int next_tail = (tail_ + 1) % kQueueCapacity;
  if (next_tail == head_) return false;
  queue_[tail_] = event;
  tail_ = next_tail;
  return true;
}

int FltkKeySource::handleFltkEvent(int event) {
  if (active_source_ == nullptr ||
      (event != FL_KEYDOWN && event != FL_KEYUP)) {
    return 0;
  }
  active_source_->onFltkEvent(event == FL_KEYDOWN ? KeyPhase::kDown
                                                   : KeyPhase::kUp);
  return 0;
}

void FltkKeySource::onFltkEvent(KeyPhase phase) {
  enqueue({phase, keyCode(Fl::event_key()), modifiers(), 0});
  if (phase != KeyPhase::kDown) return;

  uint32_t rune;
  if (decodeRune(Fl::event_text(), Fl::event_length(), &rune)) {
    enqueue({KeyPhase::kDown, KeyCode::kCharacter, modifiers(), rune});
  }
}

KeyCode FltkKeySource::keyCode(int key) {
  switch (key) {
    case FL_Tab:
      return KeyCode::kTab;
    case FL_Enter:
    case FL_KP_Enter:
      return KeyCode::kEnter;
    case ' ':
      return KeyCode::kSpace;
    case FL_Escape:
      return KeyCode::kEscape;
    case FL_BackSpace:
      return KeyCode::kBackspace;
    case FL_Delete:
      return KeyCode::kDelete;
    case FL_Up:
      return KeyCode::kUp;
    case FL_Down:
      return KeyCode::kDown;
    case FL_Left:
      return KeyCode::kLeft;
    case FL_Right:
      return KeyCode::kRight;
    case FL_Page_Up:
      return KeyCode::kPageUp;
    case FL_Page_Down:
      return KeyCode::kPageDown;
    case FL_Home:
      return KeyCode::kHome;
    case FL_End:
      return KeyCode::kEnd;
    default:
      return KeyCode::kUnknown;
  }
}

uint8_t FltkKeySource::modifiers() {
  int state = Fl::event_state();
  uint8_t result = 0;
  if (state & FL_SHIFT) result |= kKeyModifierShift;
  if (state & FL_CTRL) result |= kKeyModifierControl;
  if (state & FL_ALT) result |= kKeyModifierAlt;
  if (state & FL_META) result |= kKeyModifierMeta;
  return result;
}

bool FltkKeySource::decodeRune(const char* text, int length, uint32_t* rune) {
  if (length <= 0) return false;
  uint8_t first = static_cast<uint8_t>(text[0]);
  if (first < 0x80) {
    if (first == 0) return false;
    *rune = first;
    return true;
  }
  int continuation_count = first < 0xe0 ? 1 : first < 0xf0 ? 2 :
                           first < 0xf8 ? 3 : -1;
  if (continuation_count < 0 || length != continuation_count + 1) {
    return false;
  }
  uint32_t value = first & ((1 << (6 - continuation_count)) - 1);
  for (int i = 1; i <= continuation_count; ++i) {
    uint8_t byte = static_cast<uint8_t>(text[i]);
    if ((byte & 0xc0) != 0x80) return false;
    value = (value << 6) | (byte & 0x3f);
  }
  if ((continuation_count == 1 && value < 0x80) ||
      (continuation_count == 2 && value < 0x800) ||
      (continuation_count == 3 && value < 0x10000) || value > 0x10ffff ||
      (value >= 0xd800 && value <= 0xdfff)) {
    return false;
  }
  *rune = value;
  return true;
}

}  // namespace roo_windows::fake
