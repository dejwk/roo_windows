#include "roo_windows/material3/menu/menu.h"

#include <algorithm>
#include <cstdio>
#include <new>

#include "roo_display/ui/text_label.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/menu/menu_tokens.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {

const internal::MenuTokens& TokensFor(const ListEntryVisualContext& context) {
  if (context.variant == ListVariant::kBaseline) {
    return internal::kBaselineMenuTokens;
  }
  return internal::kExpressiveStandardMenuTokens;
}

int16_t ShortcutWidth(roo::string_view shortcut) {
  if (shortcut.empty()) return 0;
  return text_style_label_large()
      .font()
      .getHorizontalStringMetrics(shortcut,
                                  text_style_label_large().fontOptions())
      .advance();
}

}  // namespace

struct StandardMenuItem::TrailingPayload {
  roo::string_view shortcut;
  const roo_display::Drawable* icon = nullptr;
  MenuBadgeSpec badge;
  char numeric_badge[8] = {};
};

struct MenuEntry::AdornmentState {
  MenuTrailingAffordances content;
  Badge badge;
  Rect icon_bounds;
  Rect badge_anchor;
};

StandardMenuItem::StandardMenuItem(const StandardMenuItemInit& init)
    : headline_(init.headline),
      supporting_(init.supporting),
      leading_(init.leading),
      trailing_(),
      enabled_(init.enabled),
      selectable_(init.selectable),
      selected_(init.selected) {}

StandardMenuItem::~StandardMenuItem() = default;

roo::string_view StandardMenuItem::headlineText() const { return headline_; }

roo::string_view StandardMenuItem::supportingText() const {
  return supporting_;
}

Widget* StandardMenuItem::leading() { return leading_; }

const Widget* StandardMenuItem::leading() const { return leading_; }

bool StandardMenuItem::isEnabled() const { return enabled_; }

bool StandardMenuItem::isSelectable() const { return selectable_; }

bool StandardMenuItem::isSelected() const { return selected_; }

void StandardMenuItem::setSelectedFromMenu(bool selected) {
  selected_ = selected;
}

MenuTrailingAffordances StandardMenuItem::trailingAffordances() const {
  if (!trailing_) return {};
  return MenuTrailingAffordances{trailing_->shortcut, trailing_->icon,
                                 trailing_->badge};
}

void StandardMenuItem::setSelected(bool selected) { selected_ = selected; }

void StandardMenuItem::setEnabled(bool enabled) { enabled_ = enabled; }

StandardMenuItem::TrailingPayload& StandardMenuItem::ensureTrailingPayload() {
  if (!trailing_) trailing_.reset(new TrailingPayload());
  return *trailing_;
}

void StandardMenuItem::releaseEmptyTrailingPayload() {
  if (!trailing_) return;
  if (trailing_->shortcut.empty() && trailing_->icon == nullptr &&
      trailing_->badge.mode == BadgeMode::kHidden) {
    trailing_.reset();
  }
}

void StandardMenuItem::setShortcut(roo_display::StringView shortcut) {
  ensureTrailingPayload().shortcut = shortcut;
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::clearShortcut() {
  if (trailing_) trailing_->shortcut = {};
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::setBadgeDot() {
  TrailingPayload& payload = ensureTrailingPayload();
  payload.badge = MenuBadgeSpec{BadgeMode::kDot, {}};
}

void StandardMenuItem::setBadgeText(roo::string_view text) {
  TrailingPayload& payload = ensureTrailingPayload();
  payload.badge = MenuBadgeSpec{BadgeMode::kText, text};
}

void StandardMenuItem::setBadgeValue(unsigned int value) {
  TrailingPayload& payload = ensureTrailingPayload();
  if (value > 999) {
    std::snprintf(payload.numeric_badge, sizeof(payload.numeric_badge), "999+");
  } else {
    std::snprintf(payload.numeric_badge, sizeof(payload.numeric_badge), "%u",
                  value);
  }
  payload.badge = MenuBadgeSpec{BadgeMode::kText, payload.numeric_badge};
}

void StandardMenuItem::clearBadge() {
  if (trailing_) trailing_->badge = {};
  releaseEmptyTrailingPayload();
}

void StandardMenuItem::setTrailingIcon(const roo_display::Drawable* icon) {
  if (icon == nullptr) {
    clearTrailingIcon();
    return;
  }
  ensureTrailingPayload().icon = icon;
}

void StandardMenuItem::clearTrailingIcon() {
  if (trailing_) trailing_->icon = nullptr;
  releaseEmptyTrailingPayload();
}

MenuEntry::MenuEntry(ApplicationContext& context)
    : ListEntry(context), adornments_() {}

MenuEntry::~MenuEntry() { prepareForItemDestruction(); }

void MenuEntry::setMenuItem(MenuItem& item) {
  ListEntry::setItem(item);
  syncAdornments();
  ListEntryVisualContext visual = visualContext();
  visual.enabled = item.isEnabled();
  visual.selected = item.isSelected();
  setVisualContext(visual);
}

MenuItem* MenuEntry::menuItem() {
  return static_cast<MenuItem*>(ListEntry::item());
}

const MenuItem* MenuEntry::menuItem() const {
  return static_cast<const MenuItem*>(ListEntry::item());
}

bool MenuEntry::isClickable() const {
  const MenuItem* bound = menuItem();
  return bound != nullptr && bound->isEnabled();
}

void MenuEntry::prepareForItemDestruction() {
  adornments_.reset();
  ListEntry::clearItem();
}

void MenuEntry::onSingleTapUp(XDim x, YDim y) {
  ListEntry::onSingleTapUp(x, y);
}

void MenuEntry::onClicked() { ListEntry::onClicked(); }

void MenuEntry::syncAdornments() {
  const MenuItem* bound = menuItem();
  MenuTrailingAffordances content = bound == nullptr
                                        ? MenuTrailingAffordances{}
                                        : bound->trailingAffordances();
  if (content.shortcut.empty() && content.icon == nullptr &&
      content.badge.mode == BadgeMode::kHidden &&
      !(bound != nullptr && bound->isSelectable()) &&
      !(bound != nullptr && bound->hasSubmenu())) {
    adornments_.reset();
    return;
  }
  if (!adornments_) adornments_.reset(new AdornmentState());
  adornments_->content = content;
  switch (content.badge.mode) {
    case BadgeMode::kHidden:
      adornments_->badge.hide();
      break;
    case BadgeMode::kDot:
      adornments_->badge.setDot();
      break;
    case BadgeMode::kText:
      adornments_->badge.setText(content.badge.text);
      break;
  }
}

int16_t MenuEntry::trailingLaneWidth() const {
  if (!adornments_) return 0;
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  int16_t width = 0;
  int count = 0;
  const MenuItem* bound = menuItem();
  if (bound != nullptr && bound->isSelectable()) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (!adornments_->content.shortcut.empty()) {
    width += ShortcutWidth(adornments_->content.shortcut);
    ++count;
  }
  if (adornments_->content.icon != nullptr) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (adornments_->content.badge.mode != BadgeMode::kHidden) {
    width += Scaled(24);
    ++count;
  }
  if (bound != nullptr && bound->hasSubmenu()) {
    width += Scaled(tokens.icon_size_dp);
    ++count;
  }
  if (count > 1) width += (count - 1) * Scaled(tokens.trailing_gap_dp);
  return width == 0 ? 0 : width + Scaled(tokens.trailing_gap_dp);
}

Dimensions MenuEntry::onMeasure(WidthSpec width, HeightSpec height) {
  syncAdornments();
  int16_t lane = trailingLaneWidth();
  WidthSpec content_width = width;
  if (width.kind() == AT_MOST) {
    content_width =
        WidthSpec::AtMost(std::max<int16_t>(0, width.value() - lane));
  } else if (width.kind() == EXACTLY) {
    content_width =
        WidthSpec::Exactly(std::max<int16_t>(0, width.value() - lane));
  }
  Dimensions base = ListEntry::onMeasure(content_width, height);
  int16_t resolved_width = base.width() + lane;
  if (width.kind() == EXACTLY) resolved_width = width.value();
  if (width.kind() == AT_MOST)
    resolved_width = std::min(resolved_width, width.value());
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  return Dimensions(
      std::max<int16_t>(Scaled(tokens.min_width_dp), resolved_width),
      std::max<int16_t>(Scaled(tokens.min_item_height_dp), base.height()));
}

void MenuEntry::onLayout(bool changed, const Rect& rect) {
  int16_t lane = trailingLaneWidth();
  Rect content_rect(rect.xMin(), rect.yMin(),
                    lane > 0
                        ? std::max<XDim>(rect.xMin() - 1, rect.xMax() - lane)
                        : rect.xMax(),
                    rect.yMax());
  ListEntry::onLayout(changed, content_rect);

  if (!adornments_) return;
  const internal::MenuTokens& tokens = TokensFor(visualContext());
  int16_t size = Scaled(tokens.icon_size_dp);
  int16_t end = rect.width() - Scaled(tokens.horizontal_padding_dp) - 1;
  int16_t top = std::max<int16_t>(0, (rect.height() - size) / 2);
  adornments_->icon_bounds = Rect(end - size + 1, top, end, top + size - 1);
  adornments_->badge_anchor = adornments_->icon_bounds;
  adornments_->badge.layoutForIcon(adornments_->badge_anchor);
}

void MenuEntry::paintWidgetContents(PaintContext& ctx) {
  ListEntry::paintWidgetContents(ctx);
  if (!adornments_) return;

  const Theme& current_theme = theme();
  Color color = current_theme.material3Theme().color.onSurfaceVariant;
  Rect cursor = adornments_->icon_bounds;
  const MenuItem* bound = menuItem();

  if (bound != nullptr && bound->hasSubmenu()) {
    int16_t mid_x = (cursor.xMin() + cursor.xMax()) / 2;
    int16_t mid_y = (cursor.yMin() + cursor.yMax()) / 2;
    int16_t arm = std::max<int16_t>(2, Scaled(4));
    for (int16_t i = 0; i <= arm; ++i) {
      ctx.fillRect(mid_x - arm + i, mid_y - arm + i, mid_x - arm + i,
                   mid_y - arm + i, color);
      ctx.fillRect(mid_x - arm + i, mid_y + arm - i, mid_x - arm + i,
                   mid_y + arm - i, color);
    }
    ctx.addExclusion(cursor);
  } else if (adornments_->content.icon != nullptr) {
    ctx.drawTiled(*adornments_->content.icon, cursor,
                  roo_display::kCenter | roo_display::kMiddle, false);
    ctx.addExclusion(cursor);
  }

  if (bound != nullptr && bound->isSelectable() && bound->isSelected()) {
    int16_t x = cursor.xMin() - Scaled(28);
    int16_t y = (cursor.yMin() + cursor.yMax()) / 2;
    ctx.drawHLine(x, y, x + Scaled(4), color);
    ctx.drawHLine(x + Scaled(4), y, x + Scaled(9), color);
    ctx.addExclusion(Rect(x, y - Scaled(5), x + Scaled(9), y + Scaled(5)));
  }

  if (!adornments_->content.shortcut.empty()) {
    const TextStyle& style = text_style_label_large();
    roo_display::StringViewLabel label(adornments_->content.shortcut,
                                       style.font(), color,
                                       style.fontOptions());
    Rect text_bounds(Scaled(12), 0, Scaled(12) + label.extents().width() - 1,
                     height() - 1);
    ctx.drawTiled(label, text_bounds,
                  roo_display::kRight | roo_display::kMiddle, false);
    ctx.addExclusion(text_bounds);
  }

  if (adornments_->badge.visible()) {
    adornments_->badge.paint(ctx, current_theme);
  }
}

static_assert(sizeof(MenuEntry) <= sizeof(ListEntry) + sizeof(void*) + 4,
              "Phase 1 menu rows may add only pay-for-use adornment state");

}  // namespace roo_windows::material3
