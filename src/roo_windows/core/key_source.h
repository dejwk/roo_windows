#pragma once

#include <stdint.h>

namespace roo_windows {

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

/// Modifier bits carried by a `KeyEvent`.
enum KeyModifier : uint8_t {
  kKeyModifierShift = 1 << 0,
  kKeyModifierControl = 1 << 1,
  kKeyModifierAlt = 1 << 2,
  kKeyModifierMeta = 1 << 3,
};

/// One non-touch input sample.
///
/// `rune` is a valid Unicode scalar value only when `code` is
/// `KeyCode::kCharacter`; otherwise it is zero.
struct KeyEvent {
  KeyPhase phase;
  KeyCode code;
  uint8_t modifiers;
  uint32_t rune;
};

/// Non-blocking, borrowed source of keyboard or keypad input.
///
/// Implementations preserve source order, write at most `max_events` events,
/// and retain the remainder for a later call. The owning application keeps the
/// source alive for its entire lifetime.
class KeySource {
 public:
  virtual ~KeySource() = default;

  /// Copies up to `max_events` queued events into `out` and returns the count.
  virtual int drain(KeyEvent* out, int max_events) = 0;
};

}  // namespace roo_windows
