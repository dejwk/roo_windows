#include "roo_windows/material3/fab/fab.h"

#include "roo_logging.h"

namespace roo_windows::material3 {

FloatingActionButton::FloatingActionButton(ApplicationContext& context,
                                           const MonoIcon& icon, FabSize size,
                                           FabColorStyle color_style)
    : SurfaceWidget(context),
      icon_(&icon),
      size_(static_cast<uint8_t>(size)),
      color_style_(static_cast<uint8_t>(color_style)) {}

void FloatingActionButton::setIcon(const MonoIcon& icon) {
  icon_ = &icon;
  invalidateInterior();
}
void FloatingActionButton::setSize(FabSize size) {
  size_ = static_cast<uint8_t>(size);
  invalidateInterior();
  requestLayout();
}
void FloatingActionButton::setColorStyle(FabColorStyle style) {
  color_style_ = static_cast<uint8_t>(style);
  invalidateInterior();
}
ColorToken FloatingActionButton::containerRole() const {
  LOG(FATAL) << "Unimplemented: FAB container role";
  return ColorToken::kNone;
}
Color FloatingActionButton::background() const {
  LOG(FATAL) << "Unimplemented: FAB background";
  return roo_display::color::Transparent;
}
BorderStyle FloatingActionButton::getBorderStyle() const {
  LOG(FATAL) << "Unimplemented: FAB border";
  return BorderStyle();
}
uint8_t FloatingActionButton::getElevation() const {
  LOG(FATAL) << "Unimplemented: FAB elevation";
  return 0;
}
bool FloatingActionButton::usesHighlighterColor() const {
  LOG(FATAL) << "Unimplemented: FAB highlighter";
  return false;
}
Dimensions FloatingActionButton::getSuggestedMinimumDimensions() const {
  LOG(FATAL) << "Unimplemented: FAB size";
  return Dimensions(0, 0);
}
Dimensions FloatingActionButton::getNaturalDimensions() const {
  LOG(FATAL) << "Unimplemented: FAB size";
  return Dimensions(0, 0);
}
Dimensions FloatingActionButton::onMeasure(WidthSpec, HeightSpec) {
  LOG(FATAL) << "Unimplemented: FAB measure";
  return Dimensions(0, 0);
}
void FloatingActionButton::paint(PaintContext&) const {
  LOG(FATAL) << "Unimplemented: FAB paint";
}
void FloatingActionButton::notifyStateChanged(uint16_t state_diff) {
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
