#pragma once

#include <atomic>
#include <cstdint>
#include <pthread.h>

#include "roo_testing/host_event/host_event_endpoint.h"
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
  bool hasPendingEvents() const override;
  void onReadinessStart() override;
  void onReadinessStop() override;

  static int dispatchFltkEvent(int event, Fl_Window* window);
  static void installDispatcher();
  static void onHostEventReady(void* context);
  void onFltkEvent(KeyPhase phase, int key, PhysicalKey physical_key);

  static KeyCode keyCode(int key);
  static PhysicalKey physicalKey(int key);
  static uint8_t modifiers();
  static bool decodeRune(const char* text, int length, uint32_t* rune);

  KeyEvent queue_[kQueueCapacity];
  std::atomic<uint8_t> head_;
  std::atomic<uint8_t> tail_;
  roo_testing::HostEventEndpoint host_event_endpoint_;
  bool host_event_started_ = false;
  bool keys_down_[256];
  static FltkKeySource* active_source_;
  static pthread_mutex_t active_source_mutex_;
  static bool dispatcher_installed_;
};

}  // namespace roo_windows::fake
