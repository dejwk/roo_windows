#pragma once

#include <functional>
#include <memory>

#include "roo_collections/flat_small_hash_map.h"
#include "roo_windows/core/scroll_position.h"

namespace roo_windows {

class Widget;

/// Sparse application-owned storage for widget event handlers.
///
/// Only widgets that register a handler consume callback storage. The
/// dispatcher stores one handler per widget and event kind.
class WidgetEventDispatcher {
 public:
  /// Replaces the widget's interactive-change handler.
  ///
  /// Passing an empty function clears the handler.
  void setInteractiveChangeHandler(Widget& widget,
                                   std::function<void()> handler);

  /// Clears the widget's interactive-change handler, if present.
  void clearInteractiveChangeHandler(Widget& widget);

  /// Returns whether the widget has an interactive-change handler.
  bool hasInteractiveChangeHandler(const Widget& widget) const;

  /// Dispatches an interactive-change event to the widget's handler.
  ///
  /// No-op when no handler is registered.
  void dispatchInteractiveChange(Widget& widget);

  /// Receives the previous and newly applied content origins.
  using ScrollHandler =
      std::function<void(ScrollPosition previous, ScrollPosition current)>;

  /// Replaces the scroll handler; an empty function clears it.
  /// Registration may allocate. Does not invoke the handler.
  void setScrollPositionChangeHandler(Widget& widget, ScrollHandler handler);

  /// Clears the widget's scroll handler, if present.
  void clearScrollPositionChangeHandler(Widget& widget);

  /// Returns whether the widget has a scroll handler.
  bool hasScrollPositionChangeHandler(const Widget& widget) const;

  /// Invokes the scroll handler synchronously without allocating.
  /// The handler may replace or clear registrations during delivery.
  void dispatchScrollPositionChange(Widget& widget, ScrollPosition previous,
                                    ScrollPosition current);

  /// Clears all handlers currently associated with the widget.
  void clearHandlers(Widget& widget);

 private:
  friend class Widget;

  void moveHandlers(Widget& from, Widget& to);

  roo_collections::FlatSmallHashMap<const Widget*, std::function<void()>>
      interactive_change_handlers_;

  // Keep the active callable alive if a handler mutates the map during
  // delivery.
  roo_collections::FlatSmallHashMap<const Widget*,
                                    std::shared_ptr<ScrollHandler>>
      scroll_position_change_handlers_;
};

}  // namespace roo_windows