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
  int limit = std::max(0, std::min(width, height) / 2);
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
  label_ = label;
  invalidateInterior();
  requestLayout();
}
void ExtendedFloatingActionButton::setIcon(const MonoIcon& icon) {
  icon_ = &icon;
  invalidateInterior();
}
void ExtendedFloatingActionButton::setSize(ExtendedFabSize size) {
  size_ = static_cast<uint8_t>(size);
  invalidateInterior();
  requestLayout();
}
void ExtendedFloatingActionButton::setColorStyle(FabColorStyle style) {
  color_style_ = static_cast<uint8_t>(style);
  invalidateInterior();
}
void ExtendedFloatingActionButton::setExpanded(bool expanded) {
  expanded_ = expanded;
  invalidateInterior();
  requestLayout();
}
ColorToken ExtendedFloatingActionButton::containerRole() const {
  LOG(FATAL) << "Unimplemented: extended FAB container role";
  return ColorToken::kNone;
}
Color ExtendedFloatingActionButton::background() const {
  LOG(FATAL) << "Unimplemented: extended FAB background";
  return roo_display::color::Transparent;
}
BorderStyle ExtendedFloatingActionButton::getBorderStyle() const {
  LOG(FATAL) << "Unimplemented: extended FAB border";
  return BorderStyle();
}
uint8_t ExtendedFloatingActionButton::getElevation() const {
  LOG(FATAL) << "Unimplemented: extended FAB elevation";
  return 0;
}
bool ExtendedFloatingActionButton::usesHighlighterColor() const {
  LOG(FATAL) << "Unimplemented: extended FAB highlighter";
  return false;
}
Dimensions ExtendedFloatingActionButton::getSuggestedMinimumDimensions() const {
  LOG(FATAL) << "Unimplemented: extended FAB size";
  return Dimensions(0, 0);
}
Dimensions ExtendedFloatingActionButton::getNaturalDimensions() const {
  LOG(FATAL) << "Unimplemented: extended FAB size";
  return Dimensions(0, 0);
}
Dimensions ExtendedFloatingActionButton::onMeasure(WidthSpec, HeightSpec) {
  LOG(FATAL) << "Unimplemented: extended FAB measure";
  return Dimensions(0, 0);
}
void ExtendedFloatingActionButton::paint(PaintContext&) const {
  LOG(FATAL) << "Unimplemented: extended FAB paint";
}
void ExtendedFloatingActionButton::notifyStateChanged(uint16_t state_diff) {
  SurfaceWidget::notifyStateChanged(state_diff);
}

}  // namespace roo_windows::material3
