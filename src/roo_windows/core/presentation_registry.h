#pragma once

#include <cstdint>
#include <vector>

#include "roo_collections_flat_small_hash_map.h"
#include "roo_scheduler.h"

namespace roo_windows {

class ApplicationContext;
class Container;
class DisplayWindow;
class Widget;

/// Describes whether a widget is connected to a live, visible window tree.
enum class PresentationState : uint8_t {
  kDetached,
  kHidden,
  kPresented,
};

/// Reports the state delivered to an observed widget.
struct PresentationChange {
  PresentationState state;
  bool detached_since_delivery;
};

/// Delivers deferred effective-presentation changes to interested widgets.
class PresentationRegistry final : private roo_scheduler::Executable {
 public:
  explicit PresentationRegistry(ApplicationContext& context);

  /// Starts observing a widget, returning false after service shutdown.
  bool observe(Widget& widget);

  /// Stops observing a widget. This operation is idempotent.
  void unobserve(Widget& widget);

 private:
  friend class ApplicationContext;
  friend class Container;
  friend class DisplayWindow;
  friend class Widget;

  struct Entry {
    PresentationState last_state = PresentationState::kDetached;
    bool initial = true;
    bool detached_since_delivery = false;
  };

  void execute(roo_scheduler::ExecutionID id) override;
  void requestReevaluation();
  void noteSubtreeDetach(Widget& subtree);
  void deliverPendingChanges();
  void stop();

  ApplicationContext& context_;
  roo_collections::FlatSmallHashMap<Widget*, Entry> entries_;
  std::vector<Widget*> delivery_;
  roo_scheduler::ExecutionID notification_id_ = -1;
  bool pending_ = false;
  bool delivering_ = false;
  bool stopped_ = false;
};

}  // namespace roo_windows
