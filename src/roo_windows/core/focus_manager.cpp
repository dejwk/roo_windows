#include "roo_windows/core/focus_manager.h"

#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

bool FocusManager::requestFocus(Widget& widget) {
  if (!isEligible(widget)) return false;
  setFocused(&widget);
  return true;
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
  if (focused_ != nullptr) focused_->setFocused(true);
}

}  // namespace roo_windows
