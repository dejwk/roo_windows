#include "roo_windows/material3/fab/fab.h"

#include <algorithm>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/material3/internal/elevation.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {

struct FabGeometry {
  int16_t height;
  int16_t icon_slot;
  int16_t radius;
};

constexpr FabGeometry kIconGeometry[] = {
    {40, 24, 12}, {56, 24, 16}, {80, 28, 20}, {96, 32, 28}};

const FabGeometry& IconGeometry(FabSize size) {
  return kIconGeometry[static_cast<uint8_t>(size)];
}

struct ExtendedGeometry {
  FabGeometry body;
  int16_t leading;
  int16_t trailing;
  int16_t gap;
};

constexpr ExtendedGeometry kExtendedGeometry[] = {
    {{56, 24, 16}, 16, 16, 8},
    {{80, 28, 20}, 26, 26, 12},
    {{96, 32, 28}, 28, 28, 16},
};

const ExtendedGeometry& ExtendedTokens(ExtendedFabSize size) {
  return kExtendedGeometry[static_cast<uint8_t>(size)];
}

const TextStyle& ExtendedTextStyle(ExtendedFabSize size) {
  switch (size) {
    case ExtendedFabSize::kSmall:
      return text_style_title_medium();
    case ExtendedFabSize::kMedium:
      return text_style_title_large();
    case ExtendedFabSize::kLarge:
      return text_style_headline_small();
  }
  return text_style_title_medium();
}

int LabelAdvance(roo::string_view label, const TextStyle& style) {
  return label.empty()
             ? 0
             : style.font()
                   .getHorizontalStringMetrics(label, style.fontOptions())
                   .advance();
}

ColorToken ContainerRole(FabColorStyle style) {
  constexpr ColorToken kRoles[] = {
      ColorToken::kPrimaryContainer,    ColorToken::kSecondaryContainer,
      ColorToken::kTertiaryContainer,   ColorToken::kPrimary,
      ColorToken::kSecondary,           ColorToken::kTertiary,
      ColorToken::kSurfaceContainerHigh};
  return kRoles[static_cast<uint8_t>(style)];
}

Color ContainerColor(const Theme& theme, FabColorStyle style, bool enabled) {
  const ColorScheme& scheme = theme.material3Theme().color;
  if (!enabled) {
    return roo_display::AlphaBlend(scheme.surface,
                                   scheme.onSurface.withA(0x1F));
  }
  return scheme.resolve(ContainerRole(style));
}

Color ContentColor(const Theme& theme, FabColorStyle style, bool enabled) {
  const ColorScheme& scheme = theme.material3Theme().color;
  if (!enabled) {
    return roo_display::AlphaBlend(scheme.surface,
                                   scheme.onSurface.withA(0x61));
  }
  return style == FabColorStyle::kSurface
             ? scheme.primary
             : scheme.contentColorFor(ContainerRole(style));
}

uint8_t Elevation(bool enabled, bool pressed, bool focused, bool hovered) {
  if (!enabled) return internal::RendererElevation(0);
  if (pressed || focused) return internal::RendererElevation(3);
  return internal::RendererElevation(hovered ? 4 : 3);
}

uint8_t PreviousElevation(const Widget& widget, uint16_t diff) {
  bool enabled = widget.isEnabled() ^ ((diff & kWidgetEnabled) != 0);
  bool pressed = widget.isPressed() ^ ((diff & kWidgetPressed) != 0);
  bool focused = widget.isFocused() ^ ((diff & kWidgetFocused) != 0);
  bool hovered = widget.isHover() ^ ((diff & kWidgetHover) != 0);
  return Elevation(enabled, pressed, focused, hovered);
}

BorderStyle FabBorder(const FabGeometry& geometry, int width, int height) {
  int nominal = Scaled(geometry.height);
  int limit = std::max(
      0,
      std::min(width > 0 ? width : nominal, height > 0 ? height : nominal) / 2);
  int radius =
      std::min({static_cast<int>(Scaled(geometry.radius)), limit, 255});
  return BorderStyle(static_cast<uint8_t>(radius), 0);
}

void PaintIcon(const Canvas& canvas, const MonoIcon& icon, Color color,
               Rect area, int slot_size) {
  int x = area.xMin() + (area.width() - slot_size) / 2;
  int y = area.yMin() + (area.height() - slot_size) / 2;
  Rect slot(x, y, x + slot_size - 1, y + slot_size - 1);
  if (area.xMin() < slot.xMin()) {
    canvas.clearRect(area.xMin(), area.yMin(), slot.xMin() - 1, area.yMax());
  }
  if (slot.xMax() < area.xMax()) {
    canvas.clearRect(slot.xMax() + 1, area.yMin(), area.xMax(), area.yMax());
  }
  int middle_left = std::max<int>(area.xMin(), slot.xMin());
  int middle_right = std::min<int>(area.xMax(), slot.xMax());
  if (middle_left <= middle_right) {
    if (area.yMin() < slot.yMin()) {
      canvas.clearRect(middle_left, area.yMin(), middle_right, slot.yMin() - 1);
    }
    if (slot.yMax() < area.yMax()) {
      canvas.clearRect(middle_left, slot.yMax() + 1, middle_right, area.yMax());
    }
  }
  MonoIcon tinted = icon;
  tinted.color_mode().setColor(color);
  canvas.drawTiled(tinted, slot, roo_display::kCenter | roo_display::kMiddle);
}

}  // namespace

FloatingActionButton::FloatingActionButton(ApplicationContext& context,
                                           const MonoIcon& icon, FabSize size,
                                           FabColorStyle color_style)
    : SurfaceWidget(context),
      icon_(&icon),
      size_(static_cast<uint8_t>(size)),
      color_style_(static_cast<uint8_t>(color_style)) {}

void FloatingActionButton::setIcon(const MonoIcon& icon) {
  if (icon_ == &icon) return;
  icon_ = &icon;
  invalidateInterior();
}
void FloatingActionButton::setSize(FabSize size) {
  if (size_ == static_cast<uint8_t>(size)) return;
  size_ = static_cast<uint8_t>(size);
  invalidateInterior();
  requestLayout();
}
void FloatingActionButton::setColorStyle(FabColorStyle style) {
  if (color_style_ == static_cast<uint8_t>(style)) return;
  color_style_ = static_cast<uint8_t>(style);
  invalidateInterior();
}
ColorToken FloatingActionButton::containerRole() const {
  return ContainerRole(colorStyle());
}
Color FloatingActionButton::background() const {
  return ContainerColor(theme(), colorStyle(), isEnabled());
}
BorderStyle FloatingActionButton::getBorderStyle() const {
  return FabBorder(IconGeometry(size()), width(), height());
}
uint8_t FloatingActionButton::getElevation() const {
  return Elevation(isEnabled(), isPressed(), isFocused(), isHover());
}
bool FloatingActionButton::usesHighlighterColor() const {
  return colorStyle() == FabColorStyle::kSurface;
}
Dimensions FloatingActionButton::getSuggestedMinimumDimensions() const {
  int side = Scaled(IconGeometry(size()).height);
  return Dimensions(side, side);
}
Dimensions FloatingActionButton::getNaturalDimensions() const {
  return getSuggestedMinimumDimensions();
}
Dimensions FloatingActionButton::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions natural = getNaturalDimensions();
  return Dimensions(width.resolveSize(natural.width()),
                    height.resolveSize(natural.height()));
}
void FloatingActionButton::paint(PaintContext& ctx) const {
  PaintIcon(ctx.canvas(), *icon_,
            ContentColor(theme(), colorStyle(), isEnabled()), bounds(),
            Scaled(IconGeometry(size()).icon_slot));
}
void FloatingActionButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetEnabled | kWidgetPressed | kWidgetFocused |
                     kWidgetHover)) != 0) {
    uint8_t before = PreviousElevation(*this, state_diff);
    uint8_t after = getElevation();
    if (before != after && isVisible()) {
      elevationChanged(std::max(before, after));
    }
  }
  SurfaceWidget::notifyStateChanged(state_diff);
}

ExtendedFloatingActionButton::ExtendedFloatingActionButton(
    ApplicationContext& context, roo::string_view label, const MonoIcon& icon,
    ExtendedFabSize size, FabColorStyle color_style)
    : SurfaceWidget(context),
      label_(label),
      icon_(&icon),
      size_(static_cast<uint8_t>(size)),
      color_style_(static_cast<uint8_t>(color_style)),
      expanded_(true) {}
void ExtendedFloatingActionButton::setLabel(roo::string_view label) {
  if (label_.data() == label.data() && label_.size() == label.size()) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}
void ExtendedFloatingActionButton::setIcon(const MonoIcon& icon) {
  if (icon_ == &icon) return;
  icon_ = &icon;
  invalidateInterior();
}
void ExtendedFloatingActionButton::setSize(ExtendedFabSize size) {
  if (size_ == static_cast<uint8_t>(size)) return;
  size_ = static_cast<uint8_t>(size);
  invalidateInterior();
  requestLayout();
}
void ExtendedFloatingActionButton::setColorStyle(FabColorStyle style) {
  if (color_style_ == static_cast<uint8_t>(style)) return;
  color_style_ = static_cast<uint8_t>(style);
  invalidateInterior();
}
void ExtendedFloatingActionButton::setExpanded(bool expanded) {
  if (expanded_ == expanded) return;
  expanded_ = expanded;
  invalidateInterior();
  requestLayout();
}
ColorToken ExtendedFloatingActionButton::containerRole() const {
  return ContainerRole(colorStyle());
}
Color ExtendedFloatingActionButton::background() const {
  return ContainerColor(theme(), colorStyle(), isEnabled());
}
BorderStyle ExtendedFloatingActionButton::getBorderStyle() const {
  return FabBorder(ExtendedTokens(size()).body, width(), height());
}
uint8_t ExtendedFloatingActionButton::getElevation() const {
  return Elevation(isEnabled(), isPressed(), isFocused(), isHover());
}
bool ExtendedFloatingActionButton::usesHighlighterColor() const {
  return colorStyle() == FabColorStyle::kSurface;
}
Dimensions ExtendedFloatingActionButton::getSuggestedMinimumDimensions() const {
  int side = Scaled(ExtendedTokens(size()).body.height);
  return Dimensions(side, side);
}
Dimensions ExtendedFloatingActionButton::getNaturalDimensions() const {
  const ExtendedGeometry& tokens = ExtendedTokens(size());
  int side = Scaled(tokens.body.height);
  if (!expanded()) return Dimensions(side, side);
  const TextStyle& style = ExtendedTextStyle(size());
  int label_width = LabelAdvance(label_, style);
  int width = Scaled(tokens.leading) + Scaled(tokens.body.icon_slot) +
              (label_.empty() ? 0 : Scaled(tokens.gap) + label_width) +
              Scaled(tokens.trailing);
  return Dimensions(std::max(side, width), side);
}
Dimensions ExtendedFloatingActionButton::onMeasure(WidthSpec width,
                                                   HeightSpec height) {
  Dimensions natural = getNaturalDimensions();
  return Dimensions(width.resolveSize(natural.width()),
                    height.resolveSize(natural.height()));
}
void ExtendedFloatingActionButton::paint(PaintContext& ctx) const {
  const ExtendedGeometry& tokens = ExtendedTokens(size());
  const Canvas& canvas = ctx.canvas();
  Rect area = bounds();
  Color content = ContentColor(theme(), colorStyle(), isEnabled());
  int slot = Scaled(tokens.body.icon_slot);
  if (!expanded() || label_.empty()) {
    PaintIcon(canvas, *icon_, content, area, slot);
    return;
  }
  const TextStyle& style = ExtendedTextStyle(size());
  int label_width = LabelAdvance(label_, style);
  int gap = Scaled(tokens.gap);
  int cluster_width = slot + gap + label_width;
  int x = area.xMin() + (area.width() - cluster_width) / 2;
  if (area.xMin() < x) {
    canvas.clearRect(area.xMin(), area.yMin(), x - 1, area.yMax());
  }
  PaintIcon(canvas, *icon_, content,
            Rect(x, area.yMin(), x + slot - 1, area.yMax()), slot);
  x += slot;
  canvas.clearRect(x, area.yMin(), x + gap - 1, area.yMax());
  x += gap;
  roo_display::StringViewLabel text(label_, style.font(), content,
                                    style.fontOptions());
  canvas.drawTiled(text, Rect(x, area.yMin(), x + label_width - 1, area.yMax()),
                   roo_display::kCenter | roo_display::kMiddle);
  x += label_width;
  if (x <= area.xMax()) {
    canvas.clearRect(x, area.yMin(), area.xMax(), area.yMax());
  }
}
void ExtendedFloatingActionButton::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetEnabled | kWidgetPressed | kWidgetFocused |
                     kWidgetHover)) != 0) {
    uint8_t before = PreviousElevation(*this, state_diff);
    uint8_t after = getElevation();
    if (before != after && isVisible()) {
      elevationChanged(std::max(before, after));
    }
  }
  SurfaceWidget::notifyStateChanged(state_diff);
}

}  // namespace roo_windows::material3
