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

namespace {

struct DirectionalCandidate {
  Widget* widget = nullptr;
  int overlap_rank = 0;
  int32_t primary_distance = 0;
  int32_t orthogonal_gap = 0;
  int ordinal = 0;
};

Rect AbsoluteBounds(const Widget& widget) {
  XDim x = 0;
  YDim y = 0;
  widget.getAbsoluteOffset(x, y);
  return widget.bounds().translate(x, y);
}

bool IsBetter(const DirectionalCandidate& candidate,
              const DirectionalCandidate& best) {
  if (best.widget == nullptr) return true;
  if (candidate.overlap_rank != best.overlap_rank) {
    return candidate.overlap_rank < best.overlap_rank;
  }
  if (candidate.primary_distance != best.primary_distance) {
    return candidate.primary_distance < best.primary_distance;
  }
  if (candidate.orthogonal_gap != best.orthogonal_gap) {
    return candidate.orthogonal_gap < best.orthogonal_gap;
  }
  return candidate.ordinal < best.ordinal;
}

bool IsEligibleForDirection(const Widget& widget) {
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

void FindDirectionalFocusable(Widget& widget, const Rect& source,
                              FocusDirection direction,
                              int& ordinal, DirectionalCandidate& best) {
  if (IsEligibleForDirection(widget)) {
    Rect candidate_bounds = AbsoluteBounds(widget);
    int64_t source_center_x = source.xMin() + source.xMax();
    int64_t source_center_y = source.yMin() + source.yMax();
    int64_t candidate_center_x = candidate_bounds.xMin() + candidate_bounds.xMax();
    int64_t candidate_center_y = candidate_bounds.yMin() + candidate_bounds.yMax();
    bool in_half_plane = false;
    int32_t primary_distance = 0;
    int32_t orthogonal_gap = 0;
    bool overlaps = false;
    switch (direction) {
      case FocusDirection::kLeft:
        in_half_plane = candidate_center_x < source_center_x &&
                        candidate_bounds.xMin() < source.xMin();
        primary_distance = std::max<int32_t>(0, source.xMin() - candidate_bounds.xMax());
        overlaps = candidate_bounds.yMin() <= source.yMax() && candidate_bounds.yMax() >= source.yMin();
        orthogonal_gap = overlaps ? 0 : std::max(source.yMin() - candidate_bounds.yMax(), candidate_bounds.yMin() - source.yMax());
        break;
      case FocusDirection::kRight:
        in_half_plane = candidate_center_x > source_center_x &&
                        candidate_bounds.xMax() > source.xMax();
        primary_distance = std::max<int32_t>(0, candidate_bounds.xMin() - source.xMax());
        overlaps = candidate_bounds.yMin() <= source.yMax() && candidate_bounds.yMax() >= source.yMin();
        orthogonal_gap = overlaps ? 0 : std::max(source.yMin() - candidate_bounds.yMax(), candidate_bounds.yMin() - source.yMax());
        break;
      case FocusDirection::kUp:
        in_half_plane = candidate_center_y < source_center_y &&
                        candidate_bounds.yMin() < source.yMin();
        primary_distance = std::max<int32_t>(0, source.yMin() - candidate_bounds.yMax());
        overlaps = candidate_bounds.xMin() <= source.xMax() && candidate_bounds.xMax() >= source.xMin();
        orthogonal_gap = overlaps ? 0 : std::max(source.xMin() - candidate_bounds.xMax(), candidate_bounds.xMin() - source.xMax());
        break;
      case FocusDirection::kDown:
        in_half_plane = candidate_center_y > source_center_y &&
                        candidate_bounds.yMax() > source.yMax();
        primary_distance = std::max<int32_t>(0, candidate_bounds.yMin() - source.yMax());
        overlaps = candidate_bounds.xMin() <= source.xMax() && candidate_bounds.xMax() >= source.xMin();
        orthogonal_gap = overlaps ? 0 : std::max(source.xMin() - candidate_bounds.xMax(), candidate_bounds.xMin() - source.xMax());
        break;
    }
    if (in_half_plane) {
      DirectionalCandidate candidate{&widget, overlaps ? 0 : 1,
                                     primary_distance, orthogonal_gap,
                                     ordinal};
      if (IsBetter(candidate, best)) best = candidate;
    }
    ++ordinal;
  }
  for (int i = 0; i < widget.focusChildCount(); ++i) {
    Widget* child = widget.focusChildAt(i);
    if (child != nullptr) {
      FindDirectionalFocusable(*child, source, direction, ordinal, best);
    }
  }
}

}  // namespace

bool FocusManager::moveFocusDirection(Widget& root, FocusDirection direction) {
  if (focused_ == nullptr || !isDescendantOf(*focused_, root)) return false;
  DirectionalCandidate best;
  int ordinal = 0;
  FindDirectionalFocusable(root, AbsoluteBounds(*focused_), direction, ordinal,
                           best);
  return best.widget != nullptr && requestFocus(*best.widget);
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
