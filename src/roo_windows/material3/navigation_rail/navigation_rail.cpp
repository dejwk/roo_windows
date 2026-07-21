#include "roo_windows/material3/navigation_rail/navigation_rail.h"

#include <algorithm>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/material3/navigation_rail/navigation_rail_tokens.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::ClippedStringViewLabel;
using roo_display::Color;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::SmoothFilledRoundRect;

const roo_display::Font& DestinationLabelFont() { return font_button(); }

struct DestinationContentMetrics {
  int16_t icon_width;
  int16_t icon_height;
  int16_t label_width;
  int16_t label_height;
};

struct DestinationContentGeometry {
  Rect icon_bounds;
  Rect label_bounds;
  Rect content_bounds;
  Rect indicator_bounds;
};

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

Rect UnionBounds(const Rect& first, const Rect& second) {
  if (first.empty()) return second;
  if (second.empty()) return first;
  return Rect(std::min(first.xMin(), second.xMin()),
              std::min(first.yMin(), second.yMin()),
              std::max(first.xMax(), second.xMax()),
              std::max(first.yMax(), second.yMax()));
}

/// Centers a slot inside bounds, clamping it to the available target area.
Rect CenteredBounds(const Rect& bounds, int16_t width, int16_t height) {
  if (bounds.empty() || width <= 0 || height <= 0) return EmptyRect();
  width = std::min<int16_t>(width, bounds.width());
  height = std::min<int16_t>(height, bounds.height());
  const int16_t left = bounds.xMin() + (bounds.width() - width) / 2;
  const int16_t top = bounds.yMin() + (bounds.height() - height) / 2;
  return Rect(left, top, left + width - 1, top + height - 1);
}

DestinationContentMetrics ResolveContentMetrics(roo::string_view label,
                                                const MonoIcon* icon) {
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  DestinationContentMetrics result = {0, 0, 0, 0};
  if (icon != nullptr) {
    result.icon_width = std::max<int16_t>(Scaled(tokens.icon_size_dp),
                                          icon->anchorExtents().width());
    result.icon_height = std::max<int16_t>(Scaled(tokens.icon_size_dp),
                                           icon->anchorExtents().height());
  }
  if (!label.empty()) {
    result.label_width =
        DestinationLabelFont().getHorizontalStringMetrics(label).width();
    result.label_height = DestinationLabelFont().metrics().maxHeight() + 1;
  }
  return result;
}

DestinationContentGeometry ResolveContentGeometry(
    const NavigationRailDestination& destination) {
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  const Rect target = destination.bounds();
  const MonoIcon* icon =
      destination.selected() && destination.selectedIcon() != nullptr
          ? destination.selectedIcon()
          : destination.icon();
  const DestinationContentMetrics metrics =
      ResolveContentMetrics(destination.label(), icon);
  DestinationContentGeometry result = {EmptyRect(), EmptyRect(), EmptyRect(),
                                       EmptyRect()};
  if (target.empty()) return result;

  if (destination.layout() == NavigationRailLayout::kCollapsed) {
    const int16_t indicator_width = std::min<int16_t>(
        target.width(), Scaled(tokens.collapsed_indicator_width_dp));
    const int16_t indicator_height = std::min<int16_t>(
        target.height(), Scaled(tokens.collapsed_indicator_height_dp));
    const int16_t indicator_top = std::max<int16_t>(
        target.yMin(), (target.yMin() + target.yMax() + 1) / 2 -
                            Scaled(tokens.destination_height_dp) / 2 +
                            Scaled(4));
    result.indicator_bounds = Rect(
        target.xMin() + (target.width() - indicator_width) / 2, indicator_top,
        target.xMin() + (target.width() - indicator_width) / 2 +
            indicator_width - 1,
        std::min<int16_t>(target.yMax(), indicator_top + indicator_height - 1));
    result.icon_bounds = CenteredBounds(result.indicator_bounds,
                                        metrics.icon_width, metrics.icon_height);
    if (metrics.label_height > 0) {
      const int16_t label_top = result.indicator_bounds.yMax() + 1 +
                                Scaled(tokens.icon_label_gap_dp);
      if (label_top <= target.yMax()) {
        result.label_bounds =
            Rect(target.xMin(), label_top, target.xMax(),
                 std::min<int16_t>(target.yMax(),
                                   label_top + metrics.label_height - 1));
      }
    }
    result.content_bounds =
        UnionBounds(result.icon_bounds, result.label_bounds);
    return result;
  }

  const int16_t gap = metrics.icon_width > 0 && metrics.label_width > 0
                          ? Scaled(tokens.icon_label_gap_dp)
                          : 0;
  const int16_t content_width =
      metrics.icon_width + gap + metrics.label_width;
  const int16_t content_height =
      std::max(metrics.icon_height, metrics.label_height);
  result.content_bounds = CenteredBounds(target, content_width, content_height);
  if (result.content_bounds.empty()) return result;

  int16_t x = result.content_bounds.xMin();
  if (metrics.icon_width > 0) {
    result.icon_bounds = CenteredBounds(
        Rect(x, result.content_bounds.yMin(), x + metrics.icon_width - 1,
             result.content_bounds.yMax()),
        metrics.icon_width, metrics.icon_height);
    x += metrics.icon_width + gap;
  }
  if (metrics.label_width > 0) {
    result.label_bounds = CenteredBounds(
        Rect(x, result.content_bounds.yMin(), x + metrics.label_width - 1,
             result.content_bounds.yMax()),
        metrics.label_width, metrics.label_height);
  }

  const int16_t indicator_width = std::min<int16_t>(
      target.width(), result.content_bounds.width() +
                          2 * Scaled(tokens.expanded_indicator_padding_dp));
  const int16_t indicator_height = std::min<int16_t>(
      target.height(), Scaled(tokens.expanded_indicator_height_dp));
  result.indicator_bounds = CenteredBounds(
      Rect(result.content_bounds.xMin() -
               Scaled(tokens.expanded_indicator_padding_dp),
           target.yMin(),
           result.content_bounds.xMax() +
               Scaled(tokens.expanded_indicator_padding_dp),
           target.yMax()),
      indicator_width, indicator_height);
  return result;
}

Color ContentColorFor(const NavigationRailDestination& destination) {
  const ColorScheme& colors = destination.theme().material3Theme().color;
  if (!destination.isEnabled()) {
    return roo_display::AlphaBlend(colors.surface,
                                   colors.onSurface.withA(0x61));
  }
  return destination.selected() ? colors.onSecondaryContainer
                                : colors.onSurfaceVariant;
}

ColorToken UnselectedIndicatorRole(
    const NavigationRailDestination& destination) {
  if (destination.parent() == nullptr) return ColorToken::kSurface;
  const ColorToken role = destination.parent()->effectiveContainerRole();
  return role == ColorToken::kNone ? ColorToken::kSurface : role;
}

}  // namespace

NavigationRailDestination::NavigationRailDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : BasicWidget(context),
      label_(label),
      icon_(icon),
      selected_icon_(selected_icon),
      layout_(static_cast<uint8_t>(NavigationRailLayout::kCollapsed)),
      selected_(false),
      click_handled_on_release_(false) {}

roo::string_view NavigationRailDestination::label() const { return label_; }

void NavigationRailDestination::setLabel(roo::string_view label) {
  if (label_ == label) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationRailDestination::icon() const { return icon_; }

void NavigationRailDestination::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationRailDestination::selectedIcon() const {
  return selected_icon_;
}

void NavigationRailDestination::setSelectedIcon(const MonoIcon* icon) {
  if (selected_icon_ == icon) return;
  selected_icon_ = icon;
  invalidateInterior();
  requestLayout();
}

bool NavigationRailDestination::selected() const { return selected_ != 0; }

NavigationRailLayout NavigationRailDestination::layout() const {
  return static_cast<NavigationRailLayout>(layout_);
}

bool NavigationRailDestination::isClickable() const { return isEnabled(); }

ColorToken NavigationRailDestination::effectiveOverlayColorRole() const {
  return selected() ? ColorToken::kSecondaryContainer
                    : UnselectedIndicatorRole(*this);
}

Dimensions NavigationRailDestination::getSuggestedMinimumDimensions() const {
  const MonoIcon* content_icon =
      selected() && selectedIcon() != nullptr ? selectedIcon() : icon();
  const DestinationContentMetrics metrics =
      ResolveContentMetrics(label(), content_icon);
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  if (layout() == NavigationRailLayout::kCollapsed) {
    return Dimensions(
        std::max<int16_t>(Scaled(tokens.collapsed_min_width_dp),
                          std::max<int16_t>(metrics.icon_width,
                                            metrics.label_width + Scaled(16))),
        Scaled(tokens.destination_height_dp));
  }
  const int16_t gap = metrics.icon_width > 0 && metrics.label_width > 0
                          ? Scaled(tokens.icon_label_gap_dp)
                          : 0;
  return Dimensions(
      std::max<int16_t>(Scaled(tokens.expanded_min_width_dp),
                        metrics.icon_width + gap + metrics.label_width +
                            2 * Scaled(tokens.expanded_indicator_padding_dp)),
      Scaled(tokens.destination_height_dp));
}

void NavigationRailDestination::paint(PaintContext& ctx) const {
  const DestinationContentGeometry geometry = ResolveContentGeometry(*this);
  const Color content_color = ContentColorFor(*this);
  Color indicator_color =
      selected() ? theme().material3Theme().color.secondaryContainer
                 : ctx.bgcolor();
  const Color interaction_overlay = ctx.overlaySpec().base_overlay();
  const bool shows_indicator = selected() || interaction_overlay.a() != 0;
  if (interaction_overlay.a() != 0) {
    indicator_color = roo_display::AlphaBlend(indicator_color,
                                               interaction_overlay);
  }
  const Color foreground_background =
      shows_indicator ? indicator_color : ctx.bgcolor();
  const bool label_in_indicator =
      shows_indicator &&
      geometry.indicator_bounds.intersects(geometry.label_bounds);
  const Color label_background =
      label_in_indicator ? indicator_color : ctx.bgcolor();
  const MonoIcon* icon =
      selected() && selectedIcon() != nullptr ? selectedIcon() : this->icon();

  if (!geometry.icon_bounds.empty() && icon != nullptr) {
    MonoIcon tinted_icon = *icon;
    tinted_icon.color_mode().setColor(content_color);
    PaintContext icon_context = ctx.clipped(geometry.icon_bounds);
    icon_context.setBgcolor(foreground_background);
    icon_context.drawTiled(tinted_icon, geometry.icon_bounds, kCenter | kMiddle);
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

  // The target is the full-width interaction area. Settle every remaining
  // pixel only after foreground and indicator exclusions have been registered.
  ctx.clearRect(bounds());
  ctx.addExclusion(bounds());
}

void NavigationRailDestination::onSingleTapUp(XDim x, YDim y) {
  Widget::onSingleTapUp(x, y);
  // Selection ownership is added with the rail container in Phase 3. Keep the
  // guard explicit now so the destination continues to use one activation path
  // when that semantic callback is wired in.
  click_handled_on_release_ = false;
}

void NavigationRailDestination::onClicked() {
  click_handled_on_release_ = false;
  Widget::onClicked();
}

void NavigationRailDestination::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetHover | kWidgetFocused | kWidgetPressed |
                     kWidgetDragged | kWidgetClicking)) != 0) {
    invalidateInterior();
  }
  Widget::notifyStateChanged(state_diff);
}

Rect NavigationRailDestination::destinationContentBounds() const {
  return ResolveContentGeometry(*this).content_bounds;
}

Rect NavigationRailDestination::getDirectPaintExclusionBounds() const {
  return EmptyRect();
}

void NavigationRailDestination::setLayoutFromRail(NavigationRailLayout layout) {
  const uint8_t encoded = static_cast<uint8_t>(layout);
  if (layout_ == encoded) return;
  layout_ = encoded;
  invalidateInterior();
  requestLayout();
}

void NavigationRailDestination::setSelectedFromRail(bool selected) {
  if ((selected_ != 0) == selected) return;
  selected_ = selected;
  setSelected(selected);
  invalidateInterior();
  requestLayout();
}

}  // namespace material3
}  // namespace roo_windows
