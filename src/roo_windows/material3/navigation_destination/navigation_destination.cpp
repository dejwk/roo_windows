#include "roo_windows/material3/navigation_destination/navigation_destination.h"

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/container.h"

namespace roo_windows {
namespace material3 {
namespace internal {
namespace {

using roo_display::ClippedStringViewLabel;
using roo_display::Color;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::SmoothFilledRoundRect;

const roo_display::Font& DestinationLabelFont() { return font_button(); }

Color ContentColorFor(const NavigationDestinationBase& destination) {
  const ColorScheme& colors = destination.theme().material3Theme().color;
  if (!destination.isEnabled()) {
    return roo_display::AlphaBlend(colors.surface,
                                   colors.onSurface.withA(0x61));
  }
  return destination.selected() ? colors.onSecondaryContainer
                                : colors.onSurfaceVariant;
}

ColorToken UnselectedIndicatorRole(
    const NavigationDestinationBase& destination) {
  if (destination.parent() == nullptr) return ColorToken::kSurface;
  const ColorToken role = destination.parent()->effectiveContainerRole();
  return role == ColorToken::kNone ? ColorToken::kSurface : role;
}

}  // namespace

NavigationDestinationBase::NavigationDestinationBase(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : BasicWidget(context),
      label_(label),
      icon_(icon),
      selected_icon_(selected_icon),
      presentation_(
          static_cast<uint8_t>(NavigationDestinationPresentation::kStacked)),
      selected_(false),
      click_handled_on_release_(false) {}

roo::string_view NavigationDestinationBase::label() const { return label_; }

void NavigationDestinationBase::setLabel(roo::string_view label) {
  if (label_ == label) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationDestinationBase::icon() const { return icon_; }

void NavigationDestinationBase::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationDestinationBase::selectedIcon() const {
  return selected_icon_;
}

void NavigationDestinationBase::setSelectedIcon(const MonoIcon* icon) {
  if (selected_icon_ == icon) return;
  selected_icon_ = icon;
  invalidateInterior();
  requestLayout();
}

bool NavigationDestinationBase::selected() const { return selected_ != 0; }

bool NavigationDestinationBase::isClickable() const { return isEnabled(); }

ColorToken NavigationDestinationBase::effectiveOverlayColorRole() const {
  return selected() ? ColorToken::kSecondaryContainer
                    : UnselectedIndicatorRole(*this);
}

void NavigationDestinationBase::paint(PaintContext& ctx) const {
  const NavigationDestinationGeometry geometry = resolveDestinationGeometry();
  const Color content_color = ContentColorFor(*this);
  Color indicator_color =
      selected() ? theme().material3Theme().color.secondaryContainer
                 : ctx.bgcolor();
  const Color interaction_overlay = ctx.overlaySpec().base_overlay();
  const bool shows_indicator = selected() || interaction_overlay.a() != 0;
  if (interaction_overlay.a() != 0) {
    indicator_color =
        roo_display::AlphaBlend(indicator_color, interaction_overlay);
  }
  const Color foreground_background =
      shows_indicator ? indicator_color : ctx.bgcolor();
  const bool label_in_indicator =
      shows_indicator &&
      geometry.indicator_bounds.intersects(geometry.label_bounds);
  const Color label_background =
      label_in_indicator ? indicator_color : ctx.bgcolor();
  const MonoIcon* icon = displayedIcon();

  // Direct framebuffer paint is foreground-first. Each tiled foreground slot
  // resolves its transparent pixels against the color that will be behind it,
  // then becomes an exclusion before any lower-z shape is drawn.
  if (!geometry.icon_bounds.empty() && icon != nullptr) {
    MonoIcon tinted_icon = *icon;
    tinted_icon.color_mode().setColor(content_color);
    PaintContext icon_context = ctx.clipped(geometry.icon_bounds);
    icon_context.setBgcolor(foreground_background);
    icon_context.drawTiled(tinted_icon, geometry.icon_bounds,
                           kCenter | kMiddle);
    ctx.addExclusion(geometry.icon_bounds);
  }

  if (!geometry.label_bounds.empty() && !label().empty()) {
    PaintContext label_context = ctx.clipped(geometry.label_bounds);
    label_context.setBgcolor(label_background);
    label_context.drawTiled(
        ClippedStringViewLabel(label(), DestinationLabelFont(), content_color),
        geometry.label_bounds, kCenter | kMiddle);
    ctx.addExclusion(geometry.label_bounds);
  }

  if (shows_indicator && !geometry.indicator_bounds.empty()) {
    const Rect& indicator = geometry.indicator_bounds;
    ctx.drawObject(SmoothFilledRoundRect(
        indicator.xMin(), indicator.yMin(), indicator.xMax(), indicator.yMax(),
        indicator.height() / 2, indicator_color));
    ctx.addExclusion(indicator);
  }

  // Settle the inherited surface everywhere not already covered above. This
  // final lower-z pass remains disjoint from foreground and indicator paint.
  ctx.clearRect(bounds());
  ctx.addExclusion(bounds());
}

const MonoIcon* NavigationDestinationBase::displayedIcon() const {
  return selected() && selectedIcon() != nullptr ? selectedIcon() : icon();
}

NavigationDestinationPresentation NavigationDestinationBase::presentation()
    const {
  return static_cast<NavigationDestinationPresentation>(presentation_);
}

void NavigationDestinationBase::setPresentation(
    NavigationDestinationPresentation presentation) {
  const uint8_t encoded = static_cast<uint8_t>(presentation);
  if (presentation_ == encoded) return;
  presentation_ = encoded;
  invalidateInterior();
  requestLayout();
}

void NavigationDestinationBase::setSelectedFromOwner(bool selected) {
  if ((selected_ != 0) == selected) return;
  selected_ = selected;
  setSelected(selected);
  invalidateInterior();
  requestLayout();
}

void NavigationDestinationBase::onSingleTapUp(XDim x, YDim y) {
  Widget::onSingleTapUp(x, y);
  if (parent() != nullptr && isEnabled()) {
    // Commit selection before the custom pill's final click frame settles.
    // onClicked() still receives the deferred framework completion signal;
    // the guard prevents a duplicate owner activation.
    click_handled_on_release_ = true;
    activateFromOwner();
  }
}

void NavigationDestinationBase::onClicked() {
  const bool click_was_handled_on_release = click_handled_on_release_ != 0;
  click_handled_on_release_ = false;
  if (parent() != nullptr && !click_was_handled_on_release) {
    activateFromOwner();
  }
  Widget::onClicked();
}

void NavigationDestinationBase::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetHover | kWidgetFocused | kWidgetPressed |
                     kWidgetDragged | kWidgetClicking)) != 0) {
    invalidateInterior();
  }
  Widget::notifyStateChanged(state_diff);
}

Rect NavigationDestinationBase::getDirectPaintExclusionBounds() const {
  return Rect(0, 0, -1, -1);
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows
