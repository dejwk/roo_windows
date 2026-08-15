#pragma once

#include <stdint.h>

namespace roo_windows {

class Task;

/// Physical or text key understood by the framework's non-touch input path.
enum class KeyCode : uint8_t {
  kUnknown,
  kTab,
  kEnter,
  kSpace,
  kEscape,
  kBack,
  kUp,
  kDown,
  kLeft,
  kRight,
  kPageUp,
  kPageDown,
  kHome,
  kEnd,
  kDelete,
  kBackspace,
  kCharacter,
};

/// Transition associated with a key event.
enum class KeyPhase : uint8_t { kDown, kUp, kRepeat };

/// USB HID Keyboard/Keypad usage identifying the physical switch.
///
/// Values intentionally match the one-byte HID usage page. `kNone` is only
/// suitable for value initialization; a KeySource must not deliver it.
enum class PhysicalKey : uint8_t {
  kNone = 0x00,
  kA = 0x04,
  kB = 0x05,
  kC = 0x06,
  kD = 0x07,
  kE = 0x08,
  kF = 0x09,
  kG = 0x0a,
  kH = 0x0b,
  kI = 0x0c,
  kJ = 0x0d,
  kK = 0x0e,
  kL = 0x0f,
  kM = 0x10,
  kN = 0x11,
  kO = 0x12,
  kP = 0x13,
  kQ = 0x14,
  kR = 0x15,
  kS = 0x16,
  kT = 0x17,
  kU = 0x18,
  kV = 0x19,
  kW = 0x1a,
  kX = 0x1b,
  kY = 0x1c,
  kZ = 0x1d,
  kDigit1 = 0x1e,
  kDigit2 = 0x1f,
  kDigit3 = 0x20,
  kDigit4 = 0x21,
  kDigit5 = 0x22,
  kDigit6 = 0x23,
  kDigit7 = 0x24,
  kDigit8 = 0x25,
  kDigit9 = 0x26,
  kDigit0 = 0x27,
  kEnter = 0x28,
  kEscape = 0x29,
  kBackspace = 0x2a,
  kTab = 0x2b,
  kSpace = 0x2c,
  kMinus = 0x2d,
  kEqual = 0x2e,
  kLeftBracket = 0x2f,
  kRightBracket = 0x30,
  kBackslash = 0x31,
  kSemicolon = 0x33,
  kApostrophe = 0x34,
  kGrave = 0x35,
  kComma = 0x36,
  kPeriod = 0x37,
  kSlash = 0x38,
  kHome = 0x4a,
  kPageUp = 0x4b,
  kDelete = 0x4c,
  kEnd = 0x4d,
  kPageDown = 0x4e,
  kRight = 0x4f,
  kLeft = 0x50,
  kDown = 0x51,
  kUp = 0x52,
  kKeypadSlash = 0x54,
  kKeypadAsterisk = 0x55,
  kKeypadMinus = 0x56,
  kKeypadPlus = 0x57,
  kKeypadEnter = 0x58,
  kKeypad1 = 0x59,
  kKeypad2 = 0x5a,
  kKeypad3 = 0x5b,
  kKeypad4 = 0x5c,
  kKeypad5 = 0x5d,
  kKeypad6 = 0x5e,
  kKeypad7 = 0x5f,
  kKeypad8 = 0x60,
  kKeypad9 = 0x61,
  kKeypad0 = 0x62,
  kKeypadPeriod = 0x63,
  kLeftControl = 0xe0,
  kLeftShift = 0xe1,
  kLeftAlt = 0xe2,
  kLeftMeta = 0xe3,
  kRightControl = 0xe4,
  kRightShift = 0xe5,
  kRightAlt = 0xe6,
  kRightMeta = 0xe7,
};

/// Modifier bits carried by a `KeyEvent`.
enum KeyModifier : uint8_t {
  kKeyModifierShift = 1 << 0,
  kKeyModifierControl = 1 << 1,
  kKeyModifierAlt = 1 << 2,
  kKeyModifierMeta = 1 << 3,
};

/// One non-touch input sample.
///
/// `physical_key` pairs all transitions from one switch. `rune` is a valid
/// Unicode scalar value only for a Character Down or Repeat; otherwise it is
/// zero.
struct KeyEvent {
  constexpr KeyEvent() = default;
  constexpr KeyEvent(KeyPhase event_phase, KeyCode event_code,
                     uint8_t event_modifiers, PhysicalKey event_physical_key,
                     uint32_t event_rune)
      : phase(event_phase),
        code(event_code),
        modifiers(event_modifiers),
        physical_key(event_physical_key),
        rune(event_rune) {}
  // Compatibility initializer for hand-written events. Production sources
  // always supply a nonzero physical key.
  constexpr KeyEvent(KeyPhase event_phase, KeyCode event_code,
                     uint8_t event_modifiers, uint32_t event_rune)
      : KeyEvent(event_phase, event_code, event_modifiers,
                 PhysicalKey::kNone, event_rune) {}

  KeyPhase phase = KeyPhase::kDown;
  KeyCode code = KeyCode::kUnknown;
  uint8_t modifiers = 0;
  PhysicalKey physical_key = PhysicalKey::kNone;
  uint32_t rune = 0;
};

static_assert(sizeof(KeyEvent) == 8, "KeyEvent must remain compact");

/// Non-blocking, borrowed source of keyboard or keypad input.
///
/// Implementations preserve source order, write at most `max_events` events,
/// and retain the remainder for a later call. The owning application keeps the
/// source alive for its entire lifetime.
class KeySource {
 public:
  virtual ~KeySource();

  /// Copies up to `max_events` queued events into `out` and returns the count.
  virtual int drain(KeyEvent* out, int max_events) = 0;

 private:
  friend class Task;
  Task* attached_task_ = nullptr;
};

}  // namespace roo_windows
