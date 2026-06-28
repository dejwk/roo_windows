#include "roo_windows/material3/tabs/tabs.h"

#include <algorithm>

#include "roo_display/color/color.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/theme.h"

using roo_display::ClippedStringViewLabel;
using roo_display::kCenter;
using roo_display::kMiddle;

namespace roo_windows {
namespace material3 {

namespace {

constexpr int16_t kLabelOnlyHeightDp = 48;
constexpr int16_t kIconAndLabelHeightDp = 64;
constexpr int16_t kHorizontalPaddingDp = 16;
constexpr int16_t kIconSizeDp = 24;
constexpr int16_t kMinTabWidthDp = 48;

// Returns the shared label face used by phase-1 tabs.
const roo_display::Font& TabLabelFont() { return font_button(); }

// Resolves active, inactive, and disabled foreground colors from the theme.
Color ContentColorFor(const Tab& tab) {
  const Theme& theme = tab.theme();
  if (!tab.isEnabled()) {
    return roo_display::AlphaBlend(theme.color.surface,
                                   theme.color.onSurface.withA(0x61));
  }
  return tab.isActivated() ? theme.color.primary : theme.color.onSurfaceVariant;
}

// Computes the token-sized icon slot while respecting larger concrete assets.
int16_t IconSlotSize(const MonoIcon* icon) {
  if (icon == nullptr) return 0;
  return std::max<int16_t>(Scaled(kIconSizeDp), icon->anchorExtents().width());
}

}  // namespace

Tab::Tab(ApplicationContext& context, roo::string_view label)
    : SurfaceWidget(context), label_(label), icon_(nullptr) {}

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
  Dimensions content = getContentMinimumDimensions();
  int16_t width = std::min<int16_t>(content.width(), this->width());
  int16_t left = (this->width() - width) / 2;
  return Rect(left, 0, left + width - 1, height() - 1);
}

void Tab::paintContent(PaintContext& ctx, const Rect& content_bounds,
                       Color content_color) const {
  if (content_bounds.empty()) return;
  const roo_display::Font& font = TabLabelFont();
  if (label_.empty() && icon_ == nullptr) {
    ctx.clear();
    return;
  }
  Rect b = bounds();
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
  paintContent(ctx, getCoreContentBounds(), ContentColorFor(*this));
}

void Tab::onClicked() {
  if (parent() != nullptr) {
    static_cast<Tabs*>(parent())->handleTabClicked(*this);
  }
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
      selected_index_(-1),
      variant_(static_cast<uint8_t>(variant)),
      mode_(static_cast<uint8_t>(mode)),
      shows_divider_(true),
      warned_scrollable_(false) {}

Tabs::~Tabs() { clearTabs(); }

void Tabs::setVariant(TabsVariant variant) {
  uint8_t encoded = static_cast<uint8_t>(variant);
  if (variant_ == encoded) return;
  variant_ = encoded;
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
  invalidateInterior();
}

Color Tabs::background() const { return theme().color.surface; }

void Tabs::paint(PaintContext& ctx) const { ctx.clear(); }

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
    invalidateInterior();
    requestLayout();
  } else {
    updateActivatedStates();
  }
}

void Tabs::clearTabs() {
  while (!tabs_.empty()) {
    Tab* tab = tabs_.back();
    tabs_.pop_back();
    detachChild(tab);
  }
  selected_index_ = -1;
}

bool Tabs::setSelectedIndex(int index, bool animate) {
  (void)animate;
  if (index < 0 || index >= tabCount() || index == selected_index_) {
    return false;
  }
  int old_index = selected_index_;
  selected_index_ = index;
  updateActivatedStates();
  invalidateInterior();
  requestLayout();
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
  int count = tabCount();
  if (count == 0) return;
  int16_t row_height = rowHeight();
  int16_t available_width = rect.width();
  int16_t x = 0;
  for (int i = 0; i < count; ++i) {
    int16_t next_x = ((int32_t)(i + 1) * available_width) / count;
    int16_t tab_width = next_x - x;
    tabs_[i]->measure(WidthSpec::Exactly(tab_width),
                      HeightSpec::Exactly(row_height));
    tabs_[i]->layout(Rect(x, 0, x + tab_width - 1, row_height - 1));
    x = next_x;
  }
}

}  // namespace material3
}  // namespace roo_windows
