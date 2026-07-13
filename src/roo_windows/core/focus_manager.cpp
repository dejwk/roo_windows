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

void FindFocusable(Widget& widget, FocusManager& manager, Widget*& first,
                   Widget*& previous, Widget*& next, Widget* focused) {
  if (widget.isFocusable() && widget.parent() != nullptr &&
      widget.isVisible() && widget.isEnabled()) {
    if (first == nullptr) first = &widget;
    if (previous == focused) next = &widget;
    previous = &widget;
  }
  for (int i = 0; i < widget.focusChildCount(); ++i) {
    Widget* child = widget.focusChildAt(i);
    if (child != nullptr) {
      FindFocusable(*child, manager, first, previous, next, focused);
    }
  }
}

}  // namespace

bool FocusManager::moveFocus(Widget& root, bool backwards) {
  (void)backwards;
  Widget* first = nullptr;
  Widget* previous = nullptr;
  Widget* next = nullptr;
  FindFocusable(root, *this, first, previous, next, focused_);
  if (first == nullptr) return false;
  return requestFocus(*((next != nullptr) ? next : first));
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

bool FocusManager::isDescendantOf(const Widget& widget, const Widget& ancestor) {
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
