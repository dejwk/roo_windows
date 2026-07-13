#include "roo_windows/fake/fltk_key_source.h"

#include <chrono>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

namespace roo_windows::fake {

FltkKeySource* FltkKeySource::active_source_ = nullptr;
bool FltkKeySource::dispatcher_installed_ = false;

namespace {
bool dispatching = false;
}

FltkKeySource::FltkKeySource()
    : head_(0),
      tail_(0),
      active_fltk_key_(0),
      key_is_down_(false),
      last_repeat_millis_(0) {
  pthread_mutex_init(&mutex_, nullptr);
  active_source_ = this;
}

FltkKeySource::~FltkKeySource() {
  if (active_source_ == this) active_source_ = nullptr;
  pthread_mutex_destroy(&mutex_);
}

int FltkKeySource::drain(KeyEvent* out, int max_events) {
  installDispatcher();
  pthread_mutex_lock(&mutex_);
  int count = 0;
  while (head_ != tail_ && count < max_events) {
    out[count++] = queue_[head_];
    head_ = (head_ + 1) % kQueueCapacity;
  }
  pthread_mutex_unlock(&mutex_);
  return count;
}

void FltkKeySource::installDispatcher() {
  if (dispatcher_installed_) return;
  Fl::event_dispatch(dispatchFltkEvent);
  dispatcher_installed_ = true;
}

bool FltkKeySource::enqueue(const KeyEvent& event) {
  pthread_mutex_lock(&mutex_);
  int next_tail = (tail_ + 1) % kQueueCapacity;
  bool accepted = next_tail != head_;
  if (accepted) {
    queue_[tail_] = event;
    tail_ = next_tail;
  }
  pthread_mutex_unlock(&mutex_);
  return accepted;
}

int FltkKeySource::dispatchFltkEvent(int event, Fl_Window* window) {
  if (dispatching) return window == nullptr ? 0 : Fl::handle_(event, window);
  dispatching = true;
  if (active_source_ != nullptr && (event == FL_KEYDOWN || event == FL_KEYUP)) {
    int key = Fl::event_key();
    KeyPhase phase = KeyPhase::kUp;
    if (event == FL_KEYDOWN) {
      phase = active_source_->key_is_down_ &&
                      active_source_->active_fltk_key_ == key
                  ? KeyPhase::kRepeat
                  : KeyPhase::kDown;
      active_source_->active_fltk_key_ = key;
      active_source_->key_is_down_ = true;
    } else if (active_source_->active_fltk_key_ == key) {
      active_source_->key_is_down_ = false;
    }
    active_source_->onFltkEvent(phase);
  }
  int result = window == nullptr ? 0 : Fl::handle_(event, window);
  dispatching = false;
  return result;
}

void FltkKeySource::onFltkEvent(KeyPhase phase) {
  constexpr uint64_t kRepeatIntervalMillis = 50;
  uint64_t now = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
  if (phase == KeyPhase::kRepeat) {
    if (now - last_repeat_millis_ < kRepeatIntervalMillis) return;
    last_repeat_millis_ = now;
  } else if (phase == KeyPhase::kDown) {
    last_repeat_millis_ = now;
  }
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
