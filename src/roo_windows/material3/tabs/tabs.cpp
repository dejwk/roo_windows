#include "roo_windows/material3/tabs/tabs.h"

#include <algorithm>
#include <cmath>

#include "roo_display/color/color.h"
#include "roo_display/shape/basic.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/theme.h"

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
constexpr int16_t kScrollableLeadingInsetDp = 52;
constexpr int16_t kDividerHeightPx = 1;
constexpr int16_t kIndicatorFrameMs = 10;
constexpr unsigned long kIndicatorDurationMs = 200;

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

// Returns the shared label face used by phase-1 tabs.
const roo_display::Font& TabLabelFont() { return font_button(); }

// Resolves active, inactive, and disabled foreground colors from the theme.
Color ContentColorFor(const Tab& tab) {
  const Theme& theme = tab.theme();
  const ColorScheme& colors = theme.material3Theme().color;
  if (!tab.isEnabled()) {
    return roo_display::AlphaBlend(colors.surface,
                                   colors.onSurface.withA(0x61));
  }
  if (!tab.isActivated()) return colors.onSurfaceVariant;
  const Tabs* tabs = tab.parent() == nullptr
                         ? nullptr
                         : static_cast<const Tabs*>(tab.parent());
  if (tabs != nullptr && tabs->variant() == TabsVariant::kSecondary) {
    return colors.onSurface;
  }
  return colors.primary;
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

Color Tab::background() const { return theme().material3Theme().color.surface; }

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
        roo_display::FilledRect indicator_fill(
            indicator.asBox(), theme().material3Theme().color.primary);
        ctx.drawTiled(indicator_fill, indicator_band, kNoAlign);
      }
    }
  }
}

void Tab::onSingleTapUp(XDim x, YDim y) {
  Widget::onSingleTapUp(x, y);
  if (parent() != nullptr) {
    Tabs* tabs = static_cast<Tabs*>(parent());
    if (tabs->selectionCommitMode() == TabsSelectionCommitMode::kOnRelease) {
      click_handled_on_release_ = true;
      tabs->handleTabClicked(*this);
    }
  }
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

void BadgedTab::hideBadge() {
  if (!badge_.visible()) return;
  Rect old_bounds = badge_.bounds();
  badge_.hide();
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedTab::setBadgeDot() {
  if (badge_.mode() == BadgeMode::kDot) return;
  Rect old_bounds = badge_.bounds();
  badge_.setDot();
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedTab::setBadgeText(roo::string_view text) {
  if (badge_.mode() == BadgeMode::kText && badge_.text() == text) return;
  Rect old_bounds = badge_.bounds();
  badge_.setText(text);
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedTab::setBadgeValue(unsigned int number) {
  Rect old_bounds = badge_.bounds();
  badge_.setValue(number);
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

Dimensions BadgedTab::getContentMinimumDimensions() const {
  Dimensions base = Tab::getContentMinimumDimensions();
  if (!badge_.visible()) return base;

  Rect base_bounds(0, 0, std::max<int16_t>(0, base.width()) - 1,
                   std::max<int16_t>(0, base.height()) - 1);
  Rect badge_bounds = Badge::ConservativeBounds(
      base_bounds, BadgePlacement{}, badge_.mode() == BadgeMode::kText);
  Rect combined = UnionRects(base_bounds, badge_bounds);
  if (combined.empty()) return base;
  return Dimensions(combined.width(), base.height());
}

Dimensions BadgedTab::getSuggestedMinimumDimensions() const {
  return Tab::getSuggestedMinimumDimensions();
}

void BadgedTab::paint(PaintContext& ctx) const {
  badge_.paint(ctx, theme());
  Tab::paint(ctx);
}

void BadgedTab::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  relayoutBadge();
}

Rect BadgedTab::badgeAnchorBounds() const {
  Rect core = getCoreContentBounds();
  if (core.empty()) return EmptyRect();

  if (hasIcon() && icon() != nullptr) {
    int16_t icon_extent = std::max<int16_t>(Scaled(kIconSizeDp),
                                            icon()->anchorExtents().height());
    int16_t label_height =
        label().empty() ? 0 : ((TabLabelFont().metrics().maxHeight()) + 1);
    int16_t total_height = icon_extent + label_height;
    Rect paint_bounds = getContentPaintBounds();
    int16_t top = paint_bounds.yMin() +
                  (std::max<int16_t>(0, core.height() - total_height) / 2);
    int16_t icon_width =
        std::max<int16_t>(Scaled(kIconSizeDp), icon()->anchorExtents().width());
    int16_t left = core.xMin() + (core.width() - icon_width) / 2;
    return Rect(left, top, left + icon_width - 1, top + icon_extent - 1);
  }

  return core;
}

Rect BadgedTab::conservativeBadgeBounds() const {
  if (!badge_.visible()) return EmptyRect();
  return Badge::ConservativeBounds(badgeAnchorBounds(), BadgePlacement{},
                                   badge_.mode() == BadgeMode::kText);
}

Rect BadgedTab::relayoutBadge() {
  if (!badge_.visible()) return EmptyRect();
  if (!badge_.layout(badgeAnchorBounds(), BadgePlacement{})) {
    return EmptyRect();
  }
  return badge_.bounds();
}

void BadgedTab::handleBadgeGeometryChange(const Rect& old_bounds,
                                          const Rect& new_bounds) {
  Rect repaint_bounds = UnionRects(old_bounds, new_bounds);
  if (!repaint_bounds.empty()) {
    setDirty(repaint_bounds);
    if (parent() != nullptr) {
      notifyParentInvalidatedRegion(
          repaint_bounds.translate(offsetLeft(), offsetTop()));
    }
  }
  requestLayout();
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

Color Tabs::background() const {
  return theme().material3Theme().color.surface;
}

void Tabs::paint(PaintContext& ctx) const {
  Rect divider = dividerBounds();
  if (!divider.empty()) {
    roo_display::FilledRect divider_fill(
        divider.asBox(), theme().material3Theme().color.outlineVariant);
    ctx.drawTiled(divider_fill, bounds(), kNoAlign);
    return;
  }
  ctx.clear();
}

void Tabs::setSelectionCommitMode(TabsSelectionCommitMode mode) {
  selection_commit_mode_ = static_cast<uint8_t>(mode);
}

bool Tabs::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat) {
    return false;
  }
  FocusDirection direction;
  switch (event.code) {
    case KeyCode::kLeft:
      direction = FocusDirection::kLeft;
      break;
    case KeyCode::kRight:
      direction = FocusDirection::kRight;
      break;
    default:
      return false;
  }
  // A tab strip is a horizontal focus boundary. At either end it consumes the
  // key instead of allowing generic traversal to jump into unrelated content.
  context().focus().moveFocusDirection(*this, direction);
  return true;
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
  onSelectionStateUpdated(old_index, selected_index_, animate);
  onSelectedIndexChanged(old_index, selected_index_);
  return true;
}

void Tabs::onTabInvoked(int index) { (void)index; }

void Tabs::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  (void)new_index;
}

void Tabs::onSelectionStateUpdated(int old_index, int new_index, bool animate) {
  (void)old_index;
  (void)new_index;
  (void)animate;
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

void Tabs::syncIndicatorAfterLayout() {
  if (static_cast<IndicatorAnimationState>(indicator_animation_state_) ==
      IndicatorAnimationState::kIdle) {
    snapIndicatorToSelection();
  } else {
    indicator_target_ = targetIndicatorBounds();
  }
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
  syncIndicatorAfterLayout();
}

ScrollableTabs::ScrollableTabs(ApplicationContext& context, TabsVariant variant)
    : Tabs(context, variant, TabsMode::kScrollable),
      scroll_motion_(),
      scroll_notification_id_(-1),
      scroll_x_(0),
      strip_width_(0),
      intercepted_gesture_(false) {}

ScrollableTabs::~ScrollableTabs() { cancelPendingScrollUpdate(); }

void ScrollableTabs::setMode(TabsMode mode) {
  if (this->mode() == mode) return;
  cancelPendingScrollUpdate();
  scroll_x_ = 0;
  scroll_motion_ = scroll_motion::State();
  intercepted_gesture_ = false;
  Tabs::setMode(mode);
}

Dimensions ScrollableTabs::onMeasure(WidthSpec width, HeightSpec height) {
  if (mode() != TabsMode::kScrollable) {
    return Tabs::onMeasure(width, height);
  }
  int16_t desired_width = Scaled(kScrollableLeadingInsetDp);
  int16_t desired_height = rowHeight();
  for (int i = 0; i < tabCount(); ++i) {
    Dimensions child = tabAt(i).measure(WidthSpec::Unspecified(0),
                                        HeightSpec::Exactly(desired_height));
    desired_width += child.width();
  }
  return Dimensions(width.resolveSize(desired_width),
                    height.resolveSize(desired_height));
}

void ScrollableTabs::onLayout(bool changed, const Rect& rect) {
  if (mode() != TabsMode::kScrollable) {
    Tabs::onLayout(changed, rect);
    return;
  }
  (void)changed;
  (void)rect;
  int count = tabCount();
  if (count == 0) {
    strip_width_ = 0;
    scroll_x_ = 0;
    snapIndicatorToSelection();
    return;
  }

  int16_t row_height = rowHeight();
  XDim strip_x = Scaled(kScrollableLeadingInsetDp);
  for (int i = 0; i < count; ++i) {
    Dimensions child = tabAt(i).measure(WidthSpec::Unspecified(0),
                                        HeightSpec::Exactly(row_height));
    strip_x += child.width();
  }
  strip_width_ = strip_x;
  scroll_motion::Geometry geometry = motionGeometry();
  scroll_motion::Result clamped =
      scroll_motion_.scrollTo(geometry, scroll_x_, 0, scroll_x_, 0);
  scroll_x_ = clamped.x;

  XDim x = Scaled(kScrollableLeadingInsetDp) + scroll_x_;
  for (int i = 0; i < count; ++i) {
    Dimensions child = tabAt(i).measure(WidthSpec::Unspecified(0),
                                        HeightSpec::Exactly(row_height));
    tabAt(i).layout(Rect(x, 0, x + child.width() - 1, row_height - 1));
    x += child.width();
  }
  layoutScrollableChildren();
  syncIndicatorAfterLayout();
}

void ScrollableTabs::onSelectionStateUpdated(int old_index, int new_index,
                                             bool animate) {
  (void)old_index;
  (void)new_index;
  (void)animate;
  if (mode() == TabsMode::kScrollable) revealSelectedTab();
}

bool ScrollableTabs::isTabDescendant(const Widget& descendant) const {
  for (const Widget* current = &descendant; current != nullptr;
       current = current->parent()) {
    if (current == this) return true;
  }
  return false;
}

bool ScrollableTabs::revealFocusedDescendant(Widget& descendant) {
  if (mode() != TabsMode::kScrollable || !isTabDescendant(descendant) ||
      width() <= 0 || strip_width_ <= width()) {
    return mode() == TabsMode::kScrollable && isTabDescendant(descendant);
  }

  // Tab bounds are already in this row's coordinate space. Move only far
  // enough to expose the focused tab; unlike selection reveal this must not
  // alter selectedIndex() or move the indicator.
  Rect target = descendant.parent_bounds();
  for (Widget* current = descendant.parent();
       current != nullptr && current != this; current = current->parent()) {
    target = target.translate(current->offsetLeft(), current->offsetTop());
  }
  XDim target_scroll = scroll_x_;
  if (target.xMin() < 0) {
    target_scroll -= target.xMin();
  } else if (target.xMax() >= width()) {
    target_scroll -= target.xMax() - width() + 1;
  }
  applyScrollResult(scroll_motion_.scrollTo(motionGeometry(), scroll_x_, 0,
                                            target_scroll, 0));
  return true;
}

bool ScrollableTabs::onInterceptTouchEvent(const TouchEvent& event) {
  if (mode() != TabsMode::kScrollable) return false;
  if (scroll_motion_.isAnimating()) {
    intercepted_gesture_ = true;
    return true;
  }
  if (event.type() == TouchEvent::DOWN) {
    intercepted_gesture_ = false;
    return false;
  }
  if (event.type() != TouchEvent::MOVE) return intercepted_gesture_;
  if (intercepted_gesture_) return true;

  const GestureDetector& gd = getApplication()->gesture_detector();
  if (!motionGeometry().shouldClaimDrag(gd.xTotalMoveDelta(),
                                        gd.yTotalMoveDelta())) {
    return false;
  }
  intercepted_gesture_ = true;
  return true;
}

void ScrollableTabs::onDragStart(XDim x, YDim y) {
  (void)x;
  (void)y;
  if (mode() != TabsMode::kScrollable) return;
  cancelPendingScrollUpdate();
  applyScrollResult(scroll_motion_.onDown(motionGeometry(), scroll_x_, 0));
}

void ScrollableTabs::onDrag(XDim x, YDim y, XDim dx, YDim dy) {
  (void)x;
  (void)y;
  (void)dy;
  if (mode() != TabsMode::kScrollable || !motionGeometry().canScroll()) {
    return;
  }
  applyScrollResult(
      scroll_motion_.onDrag(motionGeometry(), scroll_x_, 0, dx, 0));
}

void ScrollableTabs::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  (void)x;
  (void)y;
  (void)vy;
  if (mode() != TabsMode::kScrollable) return;
  scroll_motion::Result result =
      scroll_motion_.onFling(motionGeometry(), scroll_x_, 0, vx, 0, millis());
  applyScrollResult(result);
  if (result.needs_tick) scheduleScrollUpdate();
}

void ScrollableTabs::onDragFinished(XDim x, YDim y) {
  (void)x;
  (void)y;
  if (mode() == TabsMode::kScrollable) {
    scroll_motion::Result result =
        scroll_motion_.onTouchUp(motionGeometry(), scroll_x_, 0, millis());
    applyScrollResult(result);
    if (result.needs_tick) scheduleScrollUpdate();
  }
  intercepted_gesture_ = false;
}

void ScrollableTabs::onCancel() {
  Tabs::onCancel();
  intercepted_gesture_ = false;
}

void ScrollableTabs::execute(roo_scheduler::ExecutionID id) {
  if (id != scroll_notification_id_) {
    Tabs::execute(id);
    return;
  }
  scroll_notification_id_ = -1;
  scroll_motion::Result result =
      scroll_motion_.tick(motionGeometry(), scroll_x_, 0, millis());
  applyScrollResult(result);
  if (result.needs_tick) scheduleScrollUpdate();
}

scroll_motion::Geometry ScrollableTabs::motionGeometry() const {
  XDim min_x = width() < strip_width_ ? width() - strip_width_ : 0;
  return {min_x, 0, 0, 0, scroll_motion::Axis::kHorizontal};
}

void ScrollableTabs::applyScrollResult(const scroll_motion::Result& result) {
  if (!result.changed && scroll_x_ == result.x) return;
  scroll_x_ = result.x;
  layoutScrollableChildren();
  syncIndicatorAfterLayout();
  invalidateInterior();
}

void ScrollableTabs::layoutScrollableChildren() {
  int16_t row_height = rowHeight();
  XDim x = Scaled(kScrollableLeadingInsetDp) + scroll_x_;
  for (int i = 0; i < tabCount(); ++i) {
    Tab& tab = tabAt(i);
    XDim tab_width = tab.width();
    tab.layout(Rect(x, 0, x + tab_width - 1, row_height - 1));
    x += tab_width;
  }
}

XDim ScrollableTabs::selectedTabCenterInStrip() const {
  int selected = selectedIndex();
  if (selected < 0 || selected >= tabCount()) return 0;
  const Tab& tab = tabAt(selected);
  return tab.offsetLeft() - scroll_x_ + (tab.width() - 1) / 2;
}

void ScrollableTabs::revealSelectedTab() {
  if (selectedIndex() < 0 || width() <= 0 || strip_width_ <= width()) {
    applyScrollResult(
        scroll_motion_.scrollTo(motionGeometry(), scroll_x_, 0, 0, 0));
    return;
  }
  XDim center = selectedTabCenterInStrip();
  XDim target = width() / 2 - center;
  scroll_motion::Result result =
      scroll_motion_.scrollTo(motionGeometry(), scroll_x_, 0, target, 0);
  applyScrollResult(result);
}

void ScrollableTabs::cancelPendingScrollUpdate() {
  if (scroll_notification_id_ > 0) {
    scheduler_.cancel(scroll_notification_id_);
    scroll_notification_id_ = -1;
  }
}

void ScrollableTabs::scheduleScrollUpdate() {
  cancelPendingScrollUpdate();
  scroll_notification_id_ =
      scheduler_.scheduleAfter(roo_time::Millis(kIndicatorFrameMs), *this);
}

}  // namespace material3
}  // namespace roo_windows
