#include "roo_windows/material3/button/internal/button_group_base.h"

#include <algorithm>
#include <climits>

#include "roo_windows/core/click_animation.h"
namespace roo_windows::material3::internal {
ButtonGroupBase::ButtonGroupBase(ApplicationContext& context)
    : Container(context),
      size_(static_cast<uint8_t>(ButtonSize::kSmall)),
      shape_(static_cast<uint8_t>(ButtonShape::kRound)),
      direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)),
      morph_(static_cast<uint8_t>(ButtonShapeMorph::kDefault)) {}
ButtonGroupBase::~ButtonGroupBase() { CHECK(entries_.empty()); }
void ButtonGroupBase::setSize(ButtonSize size) {
  if (this->size() == size) return;
  invalidateDescending();
  size_ = static_cast<uint8_t>(size);
  requestLayoutDescending();
}
void ButtonGroupBase::setShape(ButtonShape shape) {
  if (this->shape() == shape) return;
  invalidateDescending();
  shape_ = static_cast<uint8_t>(shape);
  invalidateDescending();
  requestLayout();
}
void ButtonGroupBase::setShapeMorph(ButtonShapeMorph morph) {
  if (shapeMorph() == morph) return;
  invalidateDescending();
  morph_ = static_cast<uint8_t>(morph);
  invalidateDescending();
  requestLayout();
}
void ButtonGroupBase::setLayoutDirection(LayoutDirection direction) {
  if (layoutDirection() == direction) return;
  invalidateDescending();
  direction_ = static_cast<uint8_t>(direction);
  invalidateDescending();
  requestLayout();
}
bool ButtonGroupBase::addEntry(ButtonGroupEntry* entry, WidgetRef reference) {
  if (entry == nullptr) return false;
  CHECK(entry->parent() == nullptr);
  CHECK(entry->group_ == nullptr);
  CHECK(&entry->context() == &context());
  if (entryCount() == kMaxEntries) return false;
  entries_.push_back(entry);
  entry->group_ = this;
  attachChild(std::move(reference));
  entryAdded();
  requestLayoutDescending();
  invalidateDescending();
  return true;
}
void ButtonGroupBase::clearEntries() {
  for (ButtonGroupEntry* entry : entries_) {
    entry->group_ = nullptr;
    entry->Widget::setSelected(false);
  }
  while (!entries_.empty()) {
    ButtonGroupEntry* entry = entries_.back();
    entries_.pop_back();
    detachChild(entry);
  }
  requestLayout();
  invalidateInterior();
}
int ButtonGroupBase::indexOf(const ButtonGroupEntry& entry) const {
  for (int i = 0; i < entryCount(); ++i) {
    if (entries_[i] == &entry) return i;
  }
  return -1;
}
void ButtonGroupBase::projectSelection(uint32_t previous, uint32_t current) {
  for (int i = 0; i < entryCount(); ++i) {
    uint32_t bit = uint32_t{1} << i;
    if (((previous ^ current) & bit) != 0)
      entries_[i]->selectionChanged((previous & bit) != 0);
    entries_[i]->Widget::setSelected((current & bit) != 0);
  }
}
Dimensions ButtonGroupBase::measureStrip(int16_t* widths,
                                         int16_t* budgets) const {
  int32_t total = 0;
  int32_t height = 0;
  int count = 0;
  for (int i = 0; i < entryCount(); ++i) {
    ButtonGroupEntry& entry = *entries_[i];
    if (widths != nullptr) widths[i] = 0;
    if (budgets != nullptr) budgets[i] = 0;
    if (entry.isGone()) continue;
    int16_t content = 0;
    Dimensions natural =
        entry.entryDimensions(budgets == nullptr ? nullptr : &content);
    if (widths != nullptr) widths[i] = natural.width();
    if (budgets != nullptr)
      budgets[i] =
          std::max(0, natural.width() - std::min<int>(natural.width(),
                                                      content + 2 * Scaled(4)));
    total += natural.width();
    height = std::max<int32_t>(height, natural.height());
    ++count;
  }
  total += std::max(0, count - 1) * ResolveGroupGap(size(), connected());
  CHECK_LE(total, INT16_MAX);
  CHECK_LE(height, INT16_MAX);
  return {static_cast<int16_t>(total), height};
}
Dimensions ButtonGroupBase::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions natural = measureStrip(nullptr);
  return {width.resolveSize(natural.width()),
          height.resolveSize(natural.height())};
}
void ButtonGroupBase::onLayout(bool changed, const Rect& rect) {
  int16_t widths[kMaxEntries];
  Dimensions natural = measureStrip(widths);
  int count = 0;
  for (ButtonGroupEntry* entry : entries_)
    if (!entry->isGone()) ++count;
  int32_t extra = connected() ? std::max(0, width() - natural.width()) : 0;
  int32_t x = 0;
  int ordinal = 0;
  for (int i = 0; i < entryCount(); ++i) {
    ButtonGroupEntry& entry = *entries_[i];
    if (entry.isGone()) continue;
    int32_t w = widths[i] + extra / count + (ordinal < extra % count ? 1 : 0);
    int32_t left = layoutDirection() == LayoutDirection::kRightToLeft
                       ? width() - x - w
                       : x;
    CHECK_LE(w, INT16_MAX);
    CHECK_GE(left, INT16_MIN);
    CHECK_LE(left + w - 1, INT16_MAX);
    entry.measure(WidthSpec::Exactly(w), HeightSpec::Exactly(natural.height()));
    entry.layout(Rect(left, 0, left + w - 1, natural.height() - 1));
    x += w + ResolveGroupGap(size(), connected());
    ++ordinal;
  }
}
void ButtonGroupBase::entryFeedbackChanged() {
  if (connected()) return;
  invalidateInterior();
  requestLayout();
}
void ButtonGroupBase::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetEnabled) != 0) {
    for (ButtonGroupEntry* entry : entries_) {
      entry->groupEnabledChanged(entry->isEnabled() && !isEnabled());
      if (!isEnabled()) entry->Widget::onCancel();
    }
    invalidateDescending();
    if (!connected()) requestLayout();
  }
  Container::notifyStateChanged(state_diff);
}
bool ButtonGroupBase::onKeyEvent(const KeyEvent& event) {
  if (event.modifiers != 0 || event.phase == KeyPhase::kUp) return false;
  bool forward = event.code == KeyCode::kRight;
  bool backward = event.code == KeyCode::kLeft;
  bool home = event.code == KeyCode::kHome;
  bool end = event.code == KeyCode::kEnd;
  if (!forward && !backward && !home && !end) return false;
  int focused = -1;
  for (int i = 0; i < entryCount(); ++i)
    if (entries_[i]->isFocused()) focused = i;
  int step =
      (home ||
       (backward && layoutDirection() == LayoutDirection::kRightToLeft) ||
       (forward && layoutDirection() == LayoutDirection::kLeftToRight))
          ? 1
          : -1;
  int start = home ? 0 : end ? entryCount() - 1 : focused + step;
  for (int i = start; i >= 0 && i < entryCount(); i += step) {
    ButtonGroupEntry& entry = *entries_[i];
    if (entry.eligible() && !entry.bounds().empty()) {
      entry.requestFocus();
      break;
    }
  }
  return true;
}
}  // namespace roo_windows::material3::internal
