#include "roo_windows/material3/tabs/tabs.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/shape/basic.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/theme.h"

using roo_display::ClippedStringViewLabel;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::kNoAlign;

namespace roo_windows {
namespace material3 {

namespace {

constexpr int16_t kLabelOnlyHeightDp = 48;
constexpr int16_t kIconAndLabelHeightDp = 64;
constexpr int16_t kHorizontalPaddingDp = 16;
constexpr int16_t kIconSizeDp = 24;
constexpr int16_t kMinTabWidthDp = 48;
constexpr int16_t kMinIndicatorWidthDp = 24;
constexpr int16_t kIndicatorHorizontalInsetDp = 2;
constexpr int16_t kPrimaryIndicatorHeightDp = 3;
constexpr int16_t kSecondaryIndicatorHeightDp = 2;
constexpr int16_t kDividerHeightPx = 1;
constexpr int16_t kIndicatorFrameMs = 10;
constexpr unsigned long kIndicatorDurationMs = 200;

// Returns the shared label face used by phase-1 tabs.
const roo_display::Font& TabLabelFont() { return font_button(); }

// Resolves active, inactive, and disabled foreground colors from the theme.
Color ContentColorFor(const Tab& tab) {
  const Theme& theme = tab.theme();
  if (!tab.isEnabled()) {
    return roo_display::AlphaBlend(theme.color.surface,
                                   theme.color.onSurface.withA(0x61));
  }
  if (!tab.isActivated()) return theme.color.onSurfaceVariant;
  const Tabs* tabs = tab.parent() == nullptr
                         ? nullptr
                         : static_cast<const Tabs*>(tab.parent());
  if (tabs != nullptr && tabs->variant() == TabsVariant::kSecondary) {
    return theme.color.onSurface;
  }
  return theme.color.primary;
}

// Computes the token-sized icon slot while respecting larger concrete assets.
int16_t IconSlotSize(const MonoIcon* icon) {
  if (icon == nullptr) return 0;
  return std::max<int16_t>(Scaled(kIconSizeDp), icon->anchorExtents().width());
}

int16_t LerpInt(int16_t from, int16_t to, float t) {
  return static_cast<int16_t>(
      std::lround(from + (static_cast<float>(to - from) * t)));
}

Rect LerpRect(const Rect& from, const Rect& to, float t) {
  if (from.empty()) return to;
  if (to.empty()) return to;
  return Rect(
      LerpInt(from.xMin(), to.xMin(), t), LerpInt(from.yMin(), to.yMin(), t),
      LerpInt(from.xMax(), to.xMax(), t), LerpInt(from.yMax(), to.yMax(), t));
}

}  // namespace

Tab::Tab(ApplicationContext& context, roo::string_view label)
    : SurfaceWidget(context),
      label_(label),
      icon_(nullptr),
      click_handled_on_release_(false) {}

void Tab::setLabel(roo::string_view label) {
  if (label_ == label) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}

void Tab::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

Color Tab::background() const { return theme().color.surface; }

Dimensions Tab::getContentMinimumDimensions() const {
  const roo_display::Font& font = TabLabelFont();
  int16_t text_width =
      label_.empty() ? 0 : font.getHorizontalStringMetrics(label_).width();
  int16_t text_height = label_.empty() ? 0 : ((font.metrics().maxHeight()) + 1);
  int16_t icon_width = IconSlotSize(icon_);
  int16_t icon_height = icon_ == nullptr ? 0 : Scaled(kIconSizeDp);
  if (icon_ != nullptr) {
    icon_height =
        std::max<int16_t>(icon_height, icon_->anchorExtents().height());
  }
  int16_t width = std::max<int16_t>(text_width, icon_width);
  int16_t height = text_height;
  if (icon_ != nullptr) {
    height += icon_height;
  }
  return Dimensions(width, height);
}

Dimensions Tab::getSuggestedMinimumDimensions() const {
  Dimensions content = getContentMinimumDimensions();
  return Dimensions(
      std::max<int16_t>(Scaled(kMinTabWidthDp),
                        content.width() + 2 * Scaled(kHorizontalPaddingDp)),
      content.height());
}

Rect Tab::getCoreContentBounds() const {
  Rect paint_bounds = getContentPaintBounds();
  if (paint_bounds.empty()) return paint_bounds;
  Dimensions content = getContentMinimumDimensions();
  int16_t width = std::min<int16_t>(content.width(), this->width());
  int16_t left = (this->width() - width) / 2;
  int16_t content_height = paint_bounds.height();
  if (parent() != nullptr) {
    const Tabs* tabs = static_cast<const Tabs*>(parent());
    content_height -= tabs->indicatorHeight();
  }
  if (width <= 0 || content_height <= 0) return Rect(0, 0, -1, -1);
  return Rect(left, paint_bounds.yMin(), left + width - 1,
              paint_bounds.yMin() + content_height - 1);
}

Rect Tab::getContentPaintBounds() const {
  if (width() <= 0 || height() <= 0) return Rect(0, 0, -1, -1);
  int16_t paint_height = height();
  if (parent() != nullptr &&
      static_cast<const Tabs*>(parent())->showsDivider()) {
    paint_height -= kDividerHeightPx;
  }
  if (paint_height <= 0) return Rect(0, 0, -1, -1);
  return Rect(0, 0, width() - 1, paint_height - 1);
}

Rect Tab::getDirectPaintExclusionBounds() const {
  return getContentPaintBounds();
}

void Tab::paintContent(PaintContext& ctx, const Rect& content_bounds,
                       Color content_color) const {
  if (content_bounds.empty()) return;
  const roo_display::Font& font = TabLabelFont();
  if (label_.empty() && icon_ == nullptr) {
    ctx.clearRect(content_bounds);
    return;
  }
  Rect b = content_bounds;
  if (icon_ == nullptr) {
    ctx.drawTiled(ClippedStringViewLabel(label_, font, content_color), b,
                  kCenter | kMiddle);
    return;
  }

  MonoIcon ic = *icon_;
  ic.color_mode().setColor(content_color);
  int16_t icon_height =
      std::max<int16_t>(Scaled(kIconSizeDp), icon_->anchorExtents().height());
  int16_t label_height =
      label_.empty() ? 0 : ((font.metrics().maxHeight()) + 1);
  int16_t total_height = icon_height + label_height;
  int16_t top = b.yMin() + (b.height() - total_height) / 2;

  if (label_.empty()) {
    ctx.drawTiled(ic, b, kCenter | kMiddle);
    return;
  }

  if (top > b.yMin()) {
    ctx.clearRect(Rect(b.xMin(), b.yMin(), b.xMax(), top - 1));
  }
  Rect icon_bounds(b.xMin(), top, b.xMax(), top + icon_height - 1);
  ctx.drawTiled(ic, icon_bounds, kCenter | kMiddle);
  Rect label_bounds(b.xMin(), icon_bounds.yMax() + 1, b.xMax(),
                    icon_bounds.yMax() + label_height);
  ctx.drawTiled(ClippedStringViewLabel(label_, font, content_color),
                label_bounds, kCenter | kMiddle);
  if (label_bounds.yMax() < b.yMax()) {
    ctx.clearRect(Rect(b.xMin(), label_bounds.yMax() + 1, b.xMax(), b.yMax()));
  }
}

void Tab::paint(PaintContext& ctx) const {
  Rect paint_bounds = getContentPaintBounds();
  Rect core_bounds = getCoreContentBounds();
  Rect foreground_bounds = core_bounds.empty()
                               ? Rect(0, 0, -1, -1)
                               : Rect(paint_bounds.xMin(), paint_bounds.yMin(),
                                      paint_bounds.xMax(), core_bounds.yMax());
  Rect indicator(0, 0, -1, -1);
  if (parent() != nullptr) {
    const Tabs* tabs = static_cast<const Tabs*>(parent());
    indicator = tabs->indicatorPaintBoundsForTab(*this);
  }
  paintContent(ctx, foreground_bounds, ContentColorFor(*this));
  if (!paint_bounds.empty()) {
    int16_t clear_y_min = foreground_bounds.empty()
                              ? paint_bounds.yMin()
                              : foreground_bounds.yMax() + 1;
    if (clear_y_min <= paint_bounds.yMax()) {
      Rect indicator_band(paint_bounds.xMin(), clear_y_min, paint_bounds.xMax(),
                          paint_bounds.yMax());
      if (indicator.empty()) {
        ctx.clearRect(indicator_band);
      } else {
        roo_display::FilledRect indicator_fill(indicator.asBox(),
                                               theme().color.primary);
        ctx.drawTiled(indicator_fill, indicator_band, kNoAlign);
      }
    }
  }
}

bool Tab::onSingleTapUp(XDim x, YDim y) {
  bool handled = Widget::onSingleTapUp(x, y);
  if (handled && parent() != nullptr) {
    Tabs* tabs = static_cast<Tabs*>(parent());
    if (tabs->selectionCommitMode() == TabsSelectionCommitMode::kOnRelease) {
      click_handled_on_release_ = true;
      tabs->handleTabClicked(*this);
    }
  }
  return handled;
}

void Tab::onClicked() {
  if (parent() != nullptr && !click_handled_on_release_) {
    static_cast<Tabs*>(parent())->handleTabClicked(*this);
  }
  click_handled_on_release_ = false;
  Widget::onClicked();
}

BadgedTab::BadgedTab(ApplicationContext& context, roo::string_view label)
    : Tab(context, label), badge_() {}

Dimensions BadgedTab::getSuggestedMinimumDimensions() const {
  LOG(WARNING) << "Unimplemented: Material3 tab badge measurement";
  return Tab::getSuggestedMinimumDimensions();
}

void BadgedTab::paint(PaintContext& ctx) const {
  LOG(WARNING) << "Unimplemented: Material3 tab badge paint";
  Tab::paint(ctx);
}

Tabs::Tabs(ApplicationContext& context, TabsVariant variant, TabsMode mode)
    : Container(context),
      tabs_(),
      scheduler_(context.scheduler()),
      notification_id_(-1),
      indicator_current_(0, 0, -1, -1),
      indicator_start_(0, 0, -1, -1),
      indicator_target_(0, 0, -1, -1),
      indicator_start_time_ms_(0),
      indicator_end_time_ms_(0),
      selected_index_(-1),
      variant_(static_cast<uint8_t>(variant)),
      mode_(static_cast<uint8_t>(mode)),
      shows_divider_(true),
      warned_scrollable_(false),
      indicator_animation_state_(
          static_cast<uint8_t>(IndicatorAnimationState::kIdle)),
      selection_commit_mode_(
          static_cast<uint8_t>(TabsSelectionCommitMode::kOnRelease)) {}

Tabs::~Tabs() {
  cancelPendingIndicatorUpdate();
  clearTabs();
}

void Tabs::setVariant(TabsVariant variant) {
  uint8_t encoded = static_cast<uint8_t>(variant);
  if (variant_ == encoded) return;
  variant_ = encoded;
  snapIndicatorToSelection();
  invalidateInterior();
}

void Tabs::setMode(TabsMode mode) {
  uint8_t encoded = static_cast<uint8_t>(mode);
  if (mode_ == encoded) return;
  mode_ = encoded;
  warned_scrollable_ = false;
  invalidateInterior();
  requestLayout();
}

void Tabs::setShowsDivider(bool shows_divider) {
  if (shows_divider_ == shows_divider) return;
  shows_divider_ = shows_divider;
  snapIndicatorToSelection();
  invalidateInterior();
}

Color Tabs::background() const { return theme().color.surface; }

void Tabs::paint(PaintContext& ctx) const {
  Rect divider = dividerBounds();
  if (!divider.empty()) {
    roo_display::FilledRect divider_fill(divider.asBox(),
                                         theme().color.outlineVariant);
    ctx.drawTiled(divider_fill, bounds(), kNoAlign);
    return;
  }
  ctx.clear();
}

void Tabs::setSelectionCommitMode(TabsSelectionCommitMode mode) {
  selection_commit_mode_ = static_cast<uint8_t>(mode);
}

void Tabs::addTabImpl(WidgetRef tab, Tab* raw) {
  CHECK_NOTNULL(tab.get());
  CHECK_NOTNULL(raw);
  tabs_.push_back(raw);
  attachChild(std::move(tab));
  if (selected_index_ < 0) {
    // Establish the initial selected state without invoking subclass hooks.
    // During construction, companion content containers may not be populated.
    selected_index_ = 0;
    updateActivatedStates();
    snapIndicatorToSelection();
    invalidateInterior();
    requestLayout();
  } else {
    updateActivatedStates();
  }
}

void Tabs::clearTabs() {
  cancelPendingIndicatorUpdate();
  while (!tabs_.empty()) {
    Tab* tab = tabs_.back();
    tabs_.pop_back();
    detachChild(tab);
  }
  selected_index_ = -1;
  indicator_current_ = Rect(0, 0, -1, -1);
  indicator_start_ = Rect(0, 0, -1, -1);
  indicator_target_ = Rect(0, 0, -1, -1);
  indicator_animation_state_ =
      static_cast<uint8_t>(IndicatorAnimationState::kIdle);
}

bool Tabs::setSelectedIndex(int index, bool animate) {
  if (index < 0 || index >= tabCount() || index == selected_index_) {
    return false;
  }
  int old_index = selected_index_;
  Rect old_indicator = indicator_current_.empty()
                           ? indicatorBoundsForIndex(old_index)
                           : indicator_current_;
  selected_index_ = index;
  updateActivatedStates();
  startIndicatorTransition(old_indicator, targetIndicatorBounds(), animate);
  onSelectedIndexChanged(old_index, selected_index_);
  return true;
}

void Tabs::onTabInvoked(int index) { (void)index; }

void Tabs::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  (void)new_index;
}

void Tabs::handleTabClicked(const Tab& tab) {
  int index = findTabIndex(tab);
  if (index < 0) return;
  onTabInvoked(index);
  setSelectedIndex(index, true);
}

int Tabs::findTabIndex(const Tab& tab) const {
  for (int i = 0; i < tabCount(); ++i) {
    if (tabs_[i] == &tab) return i;
  }
  return -1;
}

void Tabs::updateActivatedStates() {
  for (int i = 0; i < tabCount(); ++i) {
    tabs_[i]->setActivated(i == selected_index_);
  }
}

int Tabs::rowHeight() const {
  for (const Tab* tab : tabs_) {
    if (tab->hasIcon()) {
      return Scaled(kIconAndLabelHeightDp);
    }
  }
  return Scaled(kLabelOnlyHeightDp);
}

int Tabs::indicatorHeight() const {
  return variant() == TabsVariant::kPrimary
             ? Scaled(kPrimaryIndicatorHeightDp)
             : Scaled(kSecondaryIndicatorHeightDp);
}

Rect Tabs::dividerBounds() const {
  if (!showsDivider() || width() <= 0 || height() <= 0) {
    return Rect(0, 0, -1, -1);
  }
  return Rect(0, height() - kDividerHeightPx, width() - 1, height() - 1);
}

Rect Tabs::indicatorBoundsForIndex(int index) const {
  if (index < 0 || index >= tabCount() || width() <= 0 || height() <= 0) {
    return Rect(0, 0, -1, -1);
  }
  const Tab& tab = *tabs_[index];
  int16_t h = indicatorHeight();
  int16_t y_max = height() - 1 - (showsDivider() ? kDividerHeightPx : 0);
  int16_t y_min = y_max - h + 1;
  if (y_min < 0) return Rect(0, 0, -1, -1);
  if (variant() == TabsVariant::kSecondary) {
    return Rect(tab.offsetLeft(), y_min, tab.offsetLeft() + tab.width() - 1,
                y_max);
  }
  Rect content =
      tab.getCoreContentBounds().translate(tab.offsetLeft(), tab.offsetTop());
  if (content.empty()) return Rect(0, 0, -1, -1);
  int16_t indicator_width = std::max<int16_t>(
      Scaled(kMinIndicatorWidthDp),
      content.width() - 2 * Scaled(kIndicatorHorizontalInsetDp));
  indicator_width = std::min<int16_t>(indicator_width, tab.width());
  int16_t center_x = content.xMin() + (content.width() - 1) / 2;
  int16_t x_min = center_x - (indicator_width - 1) / 2;
  int16_t x_max = x_min + indicator_width - 1;
  x_min = std::max<int16_t>(tab.offsetLeft(), x_min);
  x_max = std::min<int16_t>(tab.offsetLeft() + tab.width() - 1, x_max);
  if (x_min > x_max) return Rect(0, 0, -1, -1);
  return Rect(x_min, y_min, x_max, y_max);
}

Rect Tabs::indicatorPaintBoundsForTab(const Tab& tab) const {
  Rect tab_bounds = tab.parent_bounds();
  Rect intersect = Rect::Intersect(indicator_current_, tab_bounds);
  if (intersect.empty()) return intersect;
  return intersect.translate(-tab.offsetLeft(), -tab.offsetTop());
}

Rect Tabs::targetIndicatorBounds() const {
  return indicatorBoundsForIndex(selected_index_);
}

void Tabs::snapIndicatorToSelection() {
  cancelPendingIndicatorUpdate();
  indicator_animation_state_ =
      static_cast<uint8_t>(IndicatorAnimationState::kIdle);
  indicator_current_ = targetIndicatorBounds();
  indicator_start_ = indicator_current_;
  indicator_target_ = indicator_current_;
}

void Tabs::startIndicatorTransition(const Rect& from, const Rect& to,
                                    bool animate) {
  cancelPendingIndicatorUpdate();
  indicator_target_ = to;
  if (!animate || from.empty() || to.empty() || from == to) {
    indicator_animation_state_ =
        static_cast<uint8_t>(IndicatorAnimationState::kIdle);
    indicator_current_ = to;
    indicator_start_ = to;
    invalidateInterior();
    return;
  }

  indicator_start_ = from;
  indicator_current_ = from;
  indicator_start_time_ms_ = millis();
  indicator_end_time_ms_ = indicator_start_time_ms_ + kIndicatorDurationMs;
  indicator_animation_state_ =
      static_cast<uint8_t>(IndicatorAnimationState::kAnimating);
  invalidateInterior(Rect::Extent(from, to));
  scheduleIndicatorUpdate();
}

void Tabs::cancelPendingIndicatorUpdate() {
  if (notification_id_ > 0) {
    scheduler_.cancel(notification_id_);
    notification_id_ = -1;
  }
}

void Tabs::scheduleIndicatorUpdate() {
  cancelPendingIndicatorUpdate();
  notification_id_ =
      scheduler_.scheduleAfter(roo_time::Millis(kIndicatorFrameMs), *this);
}

void Tabs::execute(roo_scheduler::ExecutionID id) {
  (void)id;
  notification_id_ = -1;
  if (static_cast<IndicatorAnimationState>(indicator_animation_state_) !=
      IndicatorAnimationState::kAnimating) {
    return;
  }

  Rect previous = indicator_current_;
  unsigned long now = millis();
  if ((long)(now - indicator_end_time_ms_) >= 0) {
    indicator_current_ = indicator_target_;
    indicator_animation_state_ =
        static_cast<uint8_t>(IndicatorAnimationState::kIdle);
    invalidateInterior(Rect::Extent(previous, indicator_current_));
    return;
  }

  float t = (float)(now - indicator_start_time_ms_) /
            (float)(indicator_end_time_ms_ - indicator_start_time_ms_);
  t = std::max(0.0f, std::min(1.0f, t));
  float eased = 1.0f - (1.0f - t) * (1.0f - t);
  indicator_current_ = LerpRect(indicator_start_, indicator_target_, eased);
  invalidateInterior(Rect::Extent(previous, indicator_current_));
  scheduleIndicatorUpdate();
}

Dimensions Tabs::onMeasure(WidthSpec width, HeightSpec height) {
  if (mode() == TabsMode::kScrollable && !warned_scrollable_) {
    LOG(WARNING) << "Unimplemented: Material3 scrollable tabs";
    warned_scrollable_ = true;
  }
  int16_t desired_width = 0;
  int16_t desired_height = rowHeight();
  for (Tab* tab : tabs_) {
    Dimensions child = tab->measure(WidthSpec::Unspecified(0),
                                    HeightSpec::Exactly(desired_height));
    desired_width += child.width();
  }
  return Dimensions(width.resolveSize(desired_width),
                    height.resolveSize(desired_height));
}

void Tabs::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  int count = tabCount();
  if (count == 0) {
    snapIndicatorToSelection();
    return;
  }
  int16_t row_height = rowHeight();
  int16_t available_width = width();
  int16_t x = 0;
  for (int i = 0; i < count; ++i) {
    int16_t next_x = ((int32_t)(i + 1) * available_width) / count;
    int16_t tab_width = next_x - x;
    tabs_[i]->measure(WidthSpec::Exactly(tab_width),
                      HeightSpec::Exactly(row_height));
    tabs_[i]->layout(Rect(x, 0, x + tab_width - 1, row_height - 1));
    x = next_x;
  }
  if (static_cast<IndicatorAnimationState>(indicator_animation_state_) ==
      IndicatorAnimationState::kIdle) {
    snapIndicatorToSelection();
  } else {
    indicator_target_ = targetIndicatorBounds();
  }
}

}  // namespace material3
}  // namespace roo_windows
