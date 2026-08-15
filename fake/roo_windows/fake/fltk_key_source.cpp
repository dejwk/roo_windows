#include "roo_windows/fake/fltk_key_source.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

namespace roo_windows::fake {

FltkKeySource* FltkKeySource::active_source_ = nullptr;
bool FltkKeySource::dispatcher_installed_ = false;

namespace {
bool dispatching = false;

bool IsPrintablePhysicalKey(PhysicalKey key) {
  uint8_t usage = static_cast<uint8_t>(key);
  return (usage >= 0x04 && usage <= 0x38) ||
         (usage >= 0x54 && usage <= 0x63);
}
}

FltkKeySource::FltkKeySource()
    : head_(0),
      tail_(0) {
  for (bool& key_down : keys_down_) key_down = false;
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
  bool consumed = false;
  if (active_source_ != nullptr && (event == FL_KEYDOWN || event == FL_KEYUP)) {
    int key = Fl::event_key();
    PhysicalKey physical_key = physicalKey(key);
    if (physical_key == PhysicalKey::kNone) {
      dispatching = false;
      return window == nullptr ? 0 : Fl::handle_(event, window);
    }
    uint8_t index = static_cast<uint8_t>(physical_key);
    KeyPhase phase = KeyPhase::kUp;
    if (event == FL_KEYDOWN) {
      phase = active_source_->keys_down_[index] ? KeyPhase::kRepeat
                                                 : KeyPhase::kDown;
      active_source_->keys_down_[index] = true;
    } else {
      active_source_->keys_down_[index] = false;
    }
    active_source_->onFltkEvent(phase, key, physical_key);
    // Keyboard input belongs to the Roo Windows key source once normalized.
    // Passing it on would let FLTK independently interpret Escape (or Tab)
    // after the framework has queued its semantic handling.
    consumed = true;
  }
  int result =
      consumed ? 1 : (window == nullptr ? 0 : Fl::handle_(event, window));
  dispatching = false;
  return result;
}

void FltkKeySource::onFltkEvent(KeyPhase phase, int key,
                                PhysicalKey physical_key) {
  KeyCode code = keyCode(key);
  uint32_t rune = 0;
  if (phase != KeyPhase::kUp && code == KeyCode::kUnknown &&
      decodeRune(Fl::event_text(), Fl::event_length(), &rune)) {
    code = KeyCode::kCharacter;
  }
  if (phase == KeyPhase::kUp && code == KeyCode::kUnknown &&
      IsPrintablePhysicalKey(physical_key)) {
    code = KeyCode::kCharacter;
  }
  enqueue({phase, code, modifiers(), physical_key, rune});
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

PhysicalKey FltkKeySource::physicalKey(int key) {
  constexpr PhysicalKey kLetters[] = {
      PhysicalKey::kA, PhysicalKey::kB, PhysicalKey::kC, PhysicalKey::kD,
      PhysicalKey::kE, PhysicalKey::kF, PhysicalKey::kG, PhysicalKey::kH,
      PhysicalKey::kI, PhysicalKey::kJ, PhysicalKey::kK, PhysicalKey::kL,
      PhysicalKey::kM, PhysicalKey::kN, PhysicalKey::kO, PhysicalKey::kP,
      PhysicalKey::kQ, PhysicalKey::kR, PhysicalKey::kS, PhysicalKey::kT,
      PhysicalKey::kU, PhysicalKey::kV, PhysicalKey::kW, PhysicalKey::kX,
      PhysicalKey::kY, PhysicalKey::kZ,
  };
  constexpr PhysicalKey kDigits[] = {
      PhysicalKey::kDigit1, PhysicalKey::kDigit2, PhysicalKey::kDigit3,
      PhysicalKey::kDigit4, PhysicalKey::kDigit5, PhysicalKey::kDigit6,
      PhysicalKey::kDigit7, PhysicalKey::kDigit8, PhysicalKey::kDigit9,
  };
  constexpr PhysicalKey kKeypadDigits[] = {
      PhysicalKey::kKeypad1, PhysicalKey::kKeypad2, PhysicalKey::kKeypad3,
      PhysicalKey::kKeypad4, PhysicalKey::kKeypad5, PhysicalKey::kKeypad6,
      PhysicalKey::kKeypad7, PhysicalKey::kKeypad8, PhysicalKey::kKeypad9,
  };
  if (key >= 'a' && key <= 'z') {
    return kLetters[key - 'a'];
  }
  if (key >= 'A' && key <= 'Z') {
    return kLetters[key - 'A'];
  }
  if (key >= '1' && key <= '9') {
    return kDigits[key - '1'];
  }
  if (key == '0') return PhysicalKey::kDigit0;
  switch (key) {
    case '-': case '_': return PhysicalKey::kMinus;
    case '=': case '+': return PhysicalKey::kEqual;
    case '[': case '{': return PhysicalKey::kLeftBracket;
    case ']': case '}': return PhysicalKey::kRightBracket;
    case '\\': case '|': return PhysicalKey::kBackslash;
    case ';': case ':': return PhysicalKey::kSemicolon;
    case '\'': case '"': return PhysicalKey::kApostrophe;
    case '`': case '~': return PhysicalKey::kGrave;
    case ',': case '<': return PhysicalKey::kComma;
    case '.': case '>': return PhysicalKey::kPeriod;
    case '/': case '?': return PhysicalKey::kSlash;
    default: break;
  }
  if (key >= FL_KP + '1' && key <= FL_KP + '9') {
    return kKeypadDigits[key - (FL_KP + '1')];
  }
  if (key == FL_KP + '0') return PhysicalKey::kKeypad0;
  switch (key) {
    case FL_Enter: return PhysicalKey::kEnter;
    case FL_KP_Enter: return PhysicalKey::kKeypadEnter;
    case FL_Escape: return PhysicalKey::kEscape;
    case FL_BackSpace: return PhysicalKey::kBackspace;
    case FL_Tab: return PhysicalKey::kTab;
    case ' ': return PhysicalKey::kSpace;
    case FL_Delete: return PhysicalKey::kDelete;
    case FL_Home: return PhysicalKey::kHome;
    case FL_Page_Up: return PhysicalKey::kPageUp;
    case FL_End: return PhysicalKey::kEnd;
    case FL_Page_Down: return PhysicalKey::kPageDown;
    case FL_Right: return PhysicalKey::kRight;
    case FL_Left: return PhysicalKey::kLeft;
    case FL_Down: return PhysicalKey::kDown;
    case FL_Up: return PhysicalKey::kUp;
    case FL_Shift_L: return PhysicalKey::kLeftShift;
    case FL_Shift_R: return PhysicalKey::kRightShift;
    case FL_Control_L: return PhysicalKey::kLeftControl;
    case FL_Control_R: return PhysicalKey::kRightControl;
    case FL_Alt_L: return PhysicalKey::kLeftAlt;
    case FL_Alt_R: return PhysicalKey::kRightAlt;
    case FL_Meta_L: return PhysicalKey::kLeftMeta;
    case FL_Meta_R: return PhysicalKey::kRightMeta;
    default: return PhysicalKey::kNone;
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
  int continuation_count = first < 0xe0   ? 1
                           : first < 0xf0 ? 2
                           : first < 0xf8 ? 3
                                          : -1;
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
