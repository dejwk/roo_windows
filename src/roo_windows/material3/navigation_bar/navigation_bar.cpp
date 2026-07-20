#include "roo_windows/material3/navigation_bar/navigation_bar.h"

#include <algorithm>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/navigation_bar/navigation_bar_tokens.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::ClippedStringViewLabel;
using roo_display::Color;
using roo_display::kCenter;
using roo_display::kMiddle;
using roo_display::SmoothFilledRoundRect;

const roo_display::Font& DestinationLabelFont() { return font_button(); }

struct DestinationContentGeometry {
  Rect icon_bounds;
  Rect label_bounds;
  Rect indicator_bounds;
};

struct DestinationContentMetrics {
  int16_t icon_width;
  int16_t icon_height;
  int16_t label_width;
  int16_t label_height;
};

/// Resolves an icon slot large enough for both the token and concrete asset.
DestinationContentMetrics ResolveContentMetrics(roo::string_view label,
                                                const MonoIcon* icon) {
  const internal::NavigationBarTokens& tokens = internal::kNavigationBarTokens;
  DestinationContentMetrics result;
  result.icon_width = 0;
  result.icon_height = 0;
  if (icon != nullptr) {
    result.icon_width = std::max<int16_t>(Scaled(tokens.icon_size_dp),
                                          icon->anchorExtents().width());
    result.icon_height = std::max<int16_t>(Scaled(tokens.icon_size_dp),
                                           icon->anchorExtents().height());
  }
  result.label_width =
      label.empty()
          ? 0
          : DestinationLabelFont().getHorizontalStringMetrics(label).width();
  result.label_height =
      label.empty() ? 0 : DestinationLabelFont().metrics().maxHeight() + 1;
  return result;
}

/// Centers a slot of `width` and `height` inside `bounds`.
Rect CenteredBounds(const Rect& bounds, int16_t width, int16_t height) {
  if (bounds.empty() || width <= 0 || height <= 0) {
    return Rect(0, 0, -1, -1);
  }
  width = std::min<int16_t>(width, bounds.width());
  height = std::min<int16_t>(height, bounds.height());
  int16_t left = bounds.xMin() + (bounds.width() - width) / 2;
  int16_t top = bounds.yMin() + (bounds.height() - height) / 2;
  return Rect(left, top, left + width - 1, top + height - 1);
}

/// Resolves geometry for the selected icon, label, and optional indicator.
DestinationContentGeometry ResolveContentGeometry(
    const NavigationBarDestination& destination) {
  const internal::NavigationBarTokens& tokens = internal::kNavigationBarTokens;
  const Rect target = destination.bounds();
  const MonoIcon* icon =
      destination.selected() && destination.selectedIcon() != nullptr
          ? destination.selectedIcon()
          : destination.icon();
  const DestinationContentMetrics metrics =
      ResolveContentMetrics(destination.label(), icon);
  DestinationContentGeometry result = {Rect(0, 0, -1, -1), Rect(0, 0, -1, -1),
                                       Rect(0, 0, -1, -1)};
  if (target.empty()) return result;

  if (destination.layout() == NavigationBarLayout::kVertical) {
    int16_t indicator_width = std::min<int16_t>(
        Scaled(tokens.vertical_indicator_width_dp), target.width());
    int16_t indicator_height = std::min<int16_t>(
        Scaled(tokens.vertical_indicator_height_dp), target.height());
    int16_t indicator_top =
        target.yMin() +
        std::max<int16_t>(
            0, (target.height() - Scaled(tokens.vertical_height_dp)) / 2 +
                   Scaled(8));
    indicator_top =
        std::min<int16_t>(indicator_top, target.yMax() - indicator_height + 1);
    int16_t indicator_left =
        target.xMin() + (target.width() - indicator_width) / 2;
    result.indicator_bounds = Rect(indicator_left, indicator_top,
                                   indicator_left + indicator_width - 1,
                                   indicator_top + indicator_height - 1);
    result.icon_bounds = CenteredBounds(
        result.indicator_bounds, metrics.icon_width, metrics.icon_height);

    if (metrics.label_height > 0) {
      int16_t label_top =
          result.indicator_bounds.yMax() + 1 + Scaled(tokens.icon_label_gap_dp);
      if (label_top <= target.yMax()) {
        result.label_bounds =
            Rect(target.xMin(), label_top, target.xMax(),
                 std::min<int16_t>(target.yMax(),
                                   label_top + metrics.label_height - 1));
      }
    }
    return result;
  }

  int16_t gap = metrics.icon_width > 0 && metrics.label_width > 0
                    ? Scaled(tokens.icon_label_gap_dp)
                    : 0;
  int16_t cluster_width = metrics.icon_width + gap + metrics.label_width;
  int16_t cluster_height = std::max(metrics.icon_height, metrics.label_height);
  Rect cluster = CenteredBounds(target, cluster_width, cluster_height);
  if (cluster.empty()) return result;

  int16_t indicator_width = std::min<int16_t>(
      target.width(),
      cluster_width + 2 * Scaled(tokens.horizontal_indicator_padding_dp));
  int16_t indicator_height = std::min<int16_t>(
      target.height(), Scaled(tokens.horizontal_indicator_height_dp));
  result.indicator_bounds = CenteredBounds(
      Rect(cluster.xMin() - Scaled(tokens.horizontal_indicator_padding_dp),
           target.yMin(),
           cluster.xMax() + Scaled(tokens.horizontal_indicator_padding_dp),
           target.yMax()),
      indicator_width, indicator_height);

  int16_t x = cluster.xMin();
  if (metrics.icon_width > 0) {
    result.icon_bounds = CenteredBounds(
        Rect(x, cluster.yMin(), x + metrics.icon_width - 1, cluster.yMax()),
        metrics.icon_width, metrics.icon_height);
    x += metrics.icon_width + gap;
  }
  if (metrics.label_width > 0) {
    result.label_bounds = CenteredBounds(
        Rect(x, cluster.yMin(), x + metrics.label_width - 1, cluster.yMax()),
        metrics.label_width, metrics.label_height);
  }
  return result;
}

/// Resolves destination foreground colors without storing a per-instance
/// palette.
Color ContentColorFor(const NavigationBarDestination& destination) {
  const ColorScheme& colors = destination.theme().material3Theme().color;
  if (!destination.isEnabled()) {
    return roo_display::AlphaBlend(colors.surface,
                                   colors.onSurface.withA(0x61));
  }
  return destination.selected() ? colors.onSecondaryContainer
                                : colors.onSurfaceVariant;
}

/// Resolves the combined hover/focus/press state layer for the indicator pill.
Color InteractionOverlayFor(const NavigationBarDestination& destination,
                            ColorToken container_role) {
  const Material3Theme& material = destination.theme().material3Theme();
  uint16_t opacity = 0;
  if (destination.isHover()) {
    opacity +=
        material.state.resolve(container_role, InteractionState::kHover).a();
  }
  if (destination.isFocused()) {
    opacity +=
        material.state.resolve(container_role, InteractionState::kFocus).a();
  }
  if (destination.isClicking()) {
    uint8_t pressed_opacity =
        material.state.resolve(container_role, InteractionState::kPressed).a();
    const ClickAnimation* animation = destination.getClickAnimation();
    float progress = animation == nullptr ? 1.0f : animation->progress();
    opacity += static_cast<uint16_t>(pressed_opacity * progress);
  } else if (destination.isPressed()) {
    opacity +=
        material.state.resolve(container_role, InteractionState::kPressed).a();
  }
  if (destination.isDragged()) {
    opacity +=
        material.state.resolve(container_role, InteractionState::kDragged).a();
  }
  Color overlay = material.color.contentColorFor(container_role);
  overlay.set_a(std::min<uint16_t>(opacity, 255));
  return overlay;
}

/// Resolves the parent surface role used by an unselected indicator state.
ColorToken UnselectedIndicatorRole(
    const NavigationBarDestination& destination) {
  if (destination.parent() == nullptr) return ColorToken::kSurface;
  ColorToken role = destination.parent()->effectiveContainerRole();
  return role == ColorToken::kNone ? ColorToken::kSurface : role;
}

}  // namespace

NavigationBarDestination::NavigationBarDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : BasicWidget(context),
      label_(label),
      icon_(icon),
      selected_icon_(selected_icon),
      layout_(static_cast<uint8_t>(NavigationBarLayout::kVertical)),
      selected_(false) {}

roo::string_view NavigationBarDestination::label() const { return label_; }

void NavigationBarDestination::setLabel(roo::string_view label) {
  if (label_ == label) return;
  label_ = label;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationBarDestination::icon() const { return icon_; }

void NavigationBarDestination::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

const MonoIcon* NavigationBarDestination::selectedIcon() const {
  return selected_icon_;
}

void NavigationBarDestination::setSelectedIcon(const MonoIcon* icon) {
  if (selected_icon_ == icon) return;
  selected_icon_ = icon;
  invalidateInterior();
  requestLayout();
}

bool NavigationBarDestination::selected() const { return selected_ != 0; }

NavigationBarLayout NavigationBarDestination::layout() const {
  return static_cast<NavigationBarLayout>(layout_);
}

bool NavigationBarDestination::isClickable() const { return isEnabled(); }

Dimensions NavigationBarDestination::getSuggestedMinimumDimensions() const {
  const MonoIcon* content_icon =
      selected() && selectedIcon() != nullptr ? selectedIcon() : icon();
  const DestinationContentMetrics metrics =
      ResolveContentMetrics(label(), content_icon);
  const internal::NavigationBarTokens& tokens = internal::kNavigationBarTokens;
  if (layout() == NavigationBarLayout::kVertical) {
    int16_t desired_width = std::max<int16_t>(
        Scaled(tokens.vertical_min_width_dp),
        std::max<int16_t>(metrics.icon_width,
                          metrics.label_width + 2 * Scaled(8)));
    return Dimensions(desired_width, Scaled(tokens.vertical_height_dp));
  }

  int16_t gap = metrics.icon_width > 0 && metrics.label_width > 0
                    ? Scaled(tokens.icon_label_gap_dp)
                    : 0;
  return Dimensions(metrics.icon_width + gap + metrics.label_width +
                        2 * Scaled(tokens.horizontal_indicator_padding_dp),
                    Scaled(tokens.horizontal_height_dp));
}

void NavigationBarDestination::paint(PaintContext& ctx) const {
  const DestinationContentGeometry geometry = ResolveContentGeometry(*this);
  const Color content_color = ContentColorFor(*this);
  const ColorToken indicator_role = selected() ? ColorToken::kSecondaryContainer
                                               : UnselectedIndicatorRole(*this);
  Color indicator_color =
      selected() ? theme().material3Theme().color.secondaryContainer
                 : ctx.bgcolor();
  const Color interaction_overlay =
      InteractionOverlayFor(*this, indicator_role);
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
  const MonoIcon* icon =
      selected() && selectedIcon() != nullptr ? selectedIcon() : this->icon();

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
    ctx.addExclusion(geometry.indicator_bounds);
  }

  // Settle the inherited bar surface everywhere not already covered above.
  // This is deliberately the final, lower-z pass: the exclusions make it
  // disjoint from the tiled foreground and the FillMode::kExtents pill,
  // including the pill's rounded-corner background pixels.
  ctx.clearRect(bounds());
  ctx.addExclusion(bounds());
}

void NavigationBarDestination::onClicked() { Widget::onClicked(); }

void NavigationBarDestination::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & (kWidgetHover | kWidgetFocused | kWidgetPressed |
                     kWidgetDragged | kWidgetClicking)) != 0) {
    invalidateInterior();
  }
  Widget::notifyStateChanged(state_diff);
}

Rect NavigationBarDestination::iconBounds() const {
  return ResolveContentGeometry(*this).icon_bounds;
}

Rect NavigationBarDestination::getDirectPaintExclusionBounds() const {
  return Rect(0, 0, -1, -1);
}

void NavigationBarDestination::setLayoutFromBar(NavigationBarLayout layout) {
  uint8_t encoded = static_cast<uint8_t>(layout);
  if (layout_ == encoded) return;
  layout_ = encoded;
  invalidateInterior();
  requestLayout();
}

void NavigationBarDestination::setSelectedFromBar(bool selected) {
  if ((selected_ != 0) == selected) return;
  selected_ = selected;
  setSelected(selected);
  invalidateInterior();
  requestLayout();
}

}  // namespace material3
}  // namespace roo_windows
