#pragma once

#include <cstdint>
#include <pthread.h>

#include "roo_windows/core/key_source.h"

class Fl_Window;

namespace roo_windows::fake {

/// Thread-safe keyboard source backed by the active FLTK event loop.
///
/// Construct one source for an emulator process before entering its FLTK event
/// loop. `enqueue()` also permits custom host-event adapters to supply already
/// normalized events without depending on FLTK event constants.
class FltkKeySource : public KeySource {
 public:
  static constexpr int kQueueCapacity = 32;

  FltkKeySource();
  ~FltkKeySource() override;

  /// Copies up to `max_events` queued events in FIFO order.
  int drain(KeyEvent* out, int max_events) override;

  /// Adds a normalized host event when queue capacity permits.
  ///
  /// Returns false when the bounded queue is full. The event is then dropped
  /// rather than blocking the FLTK event thread.
  bool enqueue(const KeyEvent& event);

 private:
  static int dispatchFltkEvent(int event, Fl_Window* window);
  static void installDispatcher();
  void onFltkEvent(KeyPhase phase);

  static KeyCode keyCode(int key);
  static uint8_t modifiers();
  static bool decodeRune(const char* text, int length, uint32_t* rune);

  pthread_mutex_t mutex_;
  KeyEvent queue_[kQueueCapacity];
  int head_;
  int tail_;
  int active_fltk_key_;
  bool key_is_down_;
  uint64_t last_repeat_millis_;

  static FltkKeySource* active_source_;
  static bool dispatcher_installed_;
};

}  // namespace roo_windows::fake
