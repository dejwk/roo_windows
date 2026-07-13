#pragma once

namespace roo_windows {

class Widget;

/// Intrusive focus record embedded by a focus-owning UI layer.
struct FocusScope {
  Widget* root = nullptr;
  Widget* last_focused = nullptr;
  FocusScope* previous = nullptr;
};

/// Application-owned focus state and focus-target lifetime tracking.
class FocusManager {
 public:
  /// Returns the currently focused widget, or null when no widget has focus.
  Widget* focused() const { return focused_; }

  /// Attempts to move focus to an eligible, attached widget.
  bool requestFocus(Widget& widget);

  /// Clears focus when `subtree` is about to detach from its parent.
  void onSubtreeDetaching(Widget& subtree);

  /// Clears an exact focus match while a widget is being destroyed.
  void onWidgetDestroying(Widget& widget);

  /// Clears focus if `widget` is no longer eligible after a state change.
  void onWidgetEligibilityChanging(Widget& widget);

 private:
  bool isEligible(const Widget& widget) const;
  static bool isDescendantOf(const Widget& widget, const Widget& ancestor);
  void setFocused(Widget* widget);

  Widget* focused_ = nullptr;
};

}  // namespace roo_windows
