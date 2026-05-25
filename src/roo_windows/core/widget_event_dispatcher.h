#pragma once

#include <functional>

#include "roo_collections/flat_small_hash_map.h"

namespace roo_windows {

class Widget;

/// Sparse application-owned storage for widget interactive-change handlers.
///
/// Only widgets that register a handler consume callback storage. The
/// dispatcher currently models the existing one-handler-per-widget semantic of
/// `Widget::setOnInteractiveChange()`.
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

  /// Clears all handlers currently associated with the widget.
  void clearHandlers(Widget& widget);

 private:
  roo_collections::FlatSmallHashMap<const Widget*, std::function<void()>>
      interactive_change_handlers_;
};

}  // namespace roo_windows