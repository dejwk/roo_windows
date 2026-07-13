#include "roo_windows/core/focus_manager.h"

#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

bool FocusManager::requestFocus(Widget& widget) {
  if (!isEligible(widget)) return false;
  setFocused(&widget);
  return true;
}

namespace {

void FindFocusable(Widget& widget, Widget*& first, Widget*& last,
                   Widget*& before_focused, Widget*& after_focused,
                   Widget* focused, bool& found_focused) {
  if (widget.isFocusable() && widget.parent() != nullptr &&
      widget.isVisible() && widget.isEnabled()) {
    if (first == nullptr) first = &widget;
    if (last == focused) after_focused = &widget;
    if (&widget == focused) {
      before_focused = last;
      found_focused = true;
    }
    last = &widget;
  }
  for (int i = 0; i < widget.focusChildCount(); ++i) {
    Widget* child = widget.focusChildAt(i);
    if (child != nullptr) {
      FindFocusable(*child, first, last, before_focused, after_focused, focused,
                    found_focused);
    }
  }
}

}  // namespace

bool FocusManager::moveFocus(Widget& root, bool backwards) {
  Widget* first = nullptr;
  Widget* last = nullptr;
  Widget* before_focused = nullptr;
  Widget* after_focused = nullptr;
  bool found_focused = false;
  FindFocusable(root, first, last, before_focused, after_focused, focused_,
                found_focused);
  if (first == nullptr) return false;
  if (backwards) {
    return requestFocus(
        *(found_focused ? (before_focused != nullptr ? before_focused : last)
                        : last));
  }
  return requestFocus(*(found_focused
                            ? (after_focused != nullptr ? after_focused : first)
                            : first));
}

void FocusManager::onSubtreeDetaching(Widget& subtree) {
  if (focused_ != nullptr && isDescendantOf(*focused_, subtree)) {
    setFocused(nullptr);
  }
}

void FocusManager::onWidgetDestroying(Widget& widget) {
  if (focused_ == &widget) setFocused(nullptr);
}

void FocusManager::onWidgetEligibilityChanging(Widget& widget) {
  if (focused_ != nullptr && isDescendantOf(*focused_, widget)) {
    setFocused(nullptr);
  }
}

bool FocusManager::isEligible(const Widget& widget) const {
  if (!widget.isFocusable() || widget.parent() == nullptr ||
      widget.bounds().empty()) {
    return false;
  }
  for (const Widget* current = &widget; current != nullptr;
       current = current->parent()) {
    if (!current->isVisible() || !current->isEnabled()) return false;
  }
  return true;
}

bool FocusManager::isDescendantOf(const Widget& widget,
                                  const Widget& ancestor) {
  for (const Widget* current = &widget; current != nullptr;
       current = current->parent()) {
    if (current == &ancestor) return true;
  }
  return false;
}

void FocusManager::setFocused(Widget* widget) {
  if (focused_ == widget) return;
  if (focused_ != nullptr) focused_->setFocused(false);
  focused_ = widget;
  if (focused_ != nullptr) {
    focused_->setFocused(true);
    for (Widget* ancestor = focused_->parent(); ancestor != nullptr;
         ancestor = ancestor->parent()) {
      if (ancestor->revealFocusedDescendant(*focused_)) break;
    }
  }
}

}  // namespace roo_windows
