#include "roo_windows/material3/navigation_bar/navigation_bar.h"

#include <algorithm>

#include "roo_display/ui/text_label.h"
#include "roo_windows/material3/navigation_bar/navigation_bar_tokens.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows {
namespace material3 {
namespace {

const TextStyle& DestinationLabelStyle() { return text_style_label_medium(); }

using DestinationContentGeometry = internal::NavigationDestinationGeometry;

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
      label.empty() ? 0
                    : DestinationLabelStyle()
                          .font()
                          .getHorizontalStringMetrics(
                              label, DestinationLabelStyle().fontOptions())
                          .advance();
  result.label_height =
      label.empty() ? 0 : DestinationLabelStyle().lineHeight();
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

}  // namespace

NavigationBarDestination::NavigationBarDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : NavigationDestinationBase(context, label, icon, selected_icon) {}

NavigationBarLayout NavigationBarDestination::layout() const {
  return presentation() == internal::NavigationDestinationPresentation::kInline
             ? NavigationBarLayout::kHorizontal
             : NavigationBarLayout::kVertical;
}

Dimensions NavigationBarDestination::getSuggestedMinimumDimensions() const {
  const MonoIcon* content_icon = displayedIcon();
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

internal::NavigationDestinationGeometry
NavigationBarDestination::resolveDestinationGeometry() const {
  return ResolveContentGeometry(*this);
}

Rect NavigationBarDestination::iconBounds() const {
  return ResolveContentGeometry(*this).icon_bounds;
}

void NavigationBarDestination::setLayoutFromBar(NavigationBarLayout layout) {
  setPresentation(layout == NavigationBarLayout::kHorizontal
                      ? internal::NavigationDestinationPresentation::kInline
                      : internal::NavigationDestinationPresentation::kStacked);
}

void NavigationBarDestination::setSelectedFromBar(bool selected) {
  setSelectedFromOwner(selected);
}

void NavigationBarDestination::activateFromOwner() {
  static_cast<NavigationBar*>(parent())->updateSelectionFromDestination(*this);
}

BadgedNavigationBarDestination::BadgedNavigationBarDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : NavigationBarDestination(context, label, icon, selected_icon), badge_() {}

const Badge& BadgedNavigationBarDestination::badge() const { return badge_; }

void BadgedNavigationBarDestination::hideBadge() {
  if (!badge_.visible()) return;
  badge_.hide();
  invalidateInterior();
}

void BadgedNavigationBarDestination::setBadgeDot() {
  badge_.setDot();
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationBarDestination::setBadgeText(roo::string_view text) {
  badge_.setText(text);
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationBarDestination::setBadgeValue(unsigned int number) {
  badge_.setValue(number);
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationBarDestination::paint(PaintContext& ctx) const {
  // The badge is front-most. Its helper settles the badge interior and
  // registers the matching exclusion before base destination paint fills the
  // lower-z indicator and inherited bar surface underneath it.
  badge_.paint(ctx, theme());
  NavigationBarDestination::paint(ctx);
}

void BadgedNavigationBarDestination::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  relayoutBadge();
}

void BadgedNavigationBarDestination::relayoutBadge() {
  if (!badge_.visible()) return;
  const Rect anchor = iconBounds();
  if (!badge_.layout(anchor)) return;

  BadgePlacement placement;
  if (badge_.mode() == BadgeMode::kText) {
    // A text badge grows from the icon's center toward its logical top end:
    // its conceptual sharp bottom-left corner is the icon-slot center. This
    // keeps the widened badge attached to the icon rather than aligned to its
    // trailing edge or positioned below it.
    const Rect text_bounds = badge_.bounds();
    const int16_t anchor_center_x = anchor.xMin() + anchor.width() / 2;
    const int16_t anchor_center_y = anchor.yMin() + anchor.height() / 2;
    placement.horizontal_offset = static_cast<int8_t>(std::clamp<int16_t>(
        text_bounds.xMin() - anchor_center_x, INT8_MIN, INT8_MAX));
    placement.vertical_offset = static_cast<int8_t>(std::clamp<int16_t>(
        anchor_center_y - text_bounds.yMax(), INT8_MIN, INT8_MAX));
    badge_.layout(anchor, placement);
  }

  const Rect badge_bounds = badge_.bounds();
  int16_t horizontal_offset = 0;
  int16_t vertical_offset = 0;
  if (badge_bounds.xMin() < bounds().xMin()) {
    horizontal_offset = badge_bounds.xMin() - bounds().xMin();
  } else if (badge_bounds.xMax() > bounds().xMax()) {
    horizontal_offset = badge_bounds.xMax() - bounds().xMax();
  }
  if (badge_bounds.yMin() < bounds().yMin()) {
    vertical_offset = bounds().yMin() - badge_bounds.yMin();
  } else if (badge_bounds.yMax() > bounds().yMax()) {
    vertical_offset = bounds().yMax() - badge_bounds.yMax();
  }
  if (horizontal_offset == 0 && vertical_offset == 0) return;
  placement.horizontal_offset = static_cast<int8_t>(std::clamp<int16_t>(
      static_cast<int16_t>(placement.horizontal_offset) + horizontal_offset,
      INT8_MIN, INT8_MAX));
  placement.vertical_offset = static_cast<int8_t>(std::clamp<int16_t>(
      static_cast<int16_t>(placement.vertical_offset) + vertical_offset,
      INT8_MIN, INT8_MAX));
  badge_.layout(anchor, placement);
}

NavigationBar::NavigationBar(ApplicationContext& context)
    : Container(context),
      destinations_(),
      selected_index_(-1),
      layout_(static_cast<uint8_t>(NavigationBarLayout::kVertical)) {}

NavigationBar::~NavigationBar() { clear(); }

NavigationBarLayout NavigationBar::layout() const {
  return static_cast<NavigationBarLayout>(layout_);
}

void NavigationBar::setLayout(NavigationBarLayout layout) {
  const uint8_t encoded = static_cast<uint8_t>(layout);
  if (layout_ == encoded) return;
  layout_ = encoded;
  propagateLayoutToDestinations();
  invalidateInterior();
  requestLayout();
}

int NavigationBar::selectedIndex() const { return selected_index_; }

void NavigationBar::setSelectedIndex(int index) {
  if (index < 0 || index >= destinationCount() || index == selected_index_) {
    return;
  }
  const int old_index = selected_index_;
  selected_index_ = static_cast<int8_t>(index);
  for (int i = 0; i < destinationCount(); ++i) {
    destinations_[i]->setSelectedFromBar(i == selected_index_);
  }
  onSelectedIndexChanged(old_index, selected_index_);
  invalidateInterior();
}

int NavigationBar::destinationCount() const {
  return static_cast<int>(destinations_.size());
}

bool NavigationBar::add(WidgetRef destination) {
  if (destination.get() == nullptr || destinationCount() >= kMaxDestinations) {
    return false;
  }
  NavigationBarDestination* raw_destination =
      static_cast<NavigationBarDestination*>(destination.get());
  CHECK(raw_destination->parent() == nullptr);
  destinations_.push_back(raw_destination);
  attachChild(std::move(destination));
  raw_destination->setLayoutFromBar(layout());
  if (selected_index_ < 0) {
    selected_index_ = 0;
    raw_destination->setSelectedFromBar(true);
  }
  invalidateInterior();
  requestLayout();
  return true;
}

void NavigationBar::clear() {
  if (destinations_.empty()) return;
  while (!destinations_.empty()) {
    NavigationBarDestination* destination = destinations_.back();
    destinations_.pop_back();
    detachChild(destination);
  }
  selected_index_ = -1;
  invalidateInterior();
  requestLayout();
}

::roo_windows::material3::ColorToken NavigationBar::containerRole() const {
  return ColorToken::kSurface;
}

Color NavigationBar::background() const {
  return theme().material3Theme().color.surface;
}

void NavigationBar::paint(PaintContext& ctx) const { ctx.clear(); }

int NavigationBar::getChildrenCount() const { return destinationCount(); }

const Widget& NavigationBar::getChild(int idx) const {
  CHECK(idx >= 0);
  CHECK_LT(idx, destinationCount());
  return *destinations_[idx];
}

Widget& NavigationBar::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const NavigationBar&>(*this).getChild(idx));
}

Dimensions NavigationBar::onMeasure(WidthSpec width, HeightSpec height) {
  const internal::NavigationBarTokens& tokens = internal::kNavigationBarTokens;
  const int16_t bar_height = Scaled(layout() == NavigationBarLayout::kVertical
                                        ? tokens.vertical_height_dp
                                        : tokens.horizontal_height_dp);
  int16_t desired_width = 0;
  if (layout() == NavigationBarLayout::kVertical) {
    for (NavigationBarDestination* destination : destinations_) {
      Dimensions child = destination->measure(WidthSpec::Unspecified(0),
                                              HeightSpec::Exactly(bar_height));
      desired_width += child.width();
    }
  } else {
    desired_width =
        destinationCount() * Scaled(tokens.horizontal_item_width_dp);
  }
  return Dimensions(width.resolveSize(desired_width),
                    height.resolveSize(bar_height));
}

void NavigationBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  const int count = destinationCount();
  if (count == 0) return;

  propagateLayoutToDestinations();
  const int16_t available_width = width();
  const int16_t bar_height = height();
  int16_t item_width = 0;
  int16_t start_x = 0;
  if (layout() == NavigationBarLayout::kHorizontal) {
    const int16_t preferred_width =
        Scaled(internal::kNavigationBarTokens.horizontal_item_width_dp);
    const int32_t preferred_total =
        static_cast<int32_t>(count) * preferred_width;
    if (available_width >= preferred_total) {
      item_width = preferred_width;
      start_x = static_cast<int16_t>((available_width - preferred_total) / 2);
    }
  }

  // Compact mode, and horizontal mode under width pressure, divide the row
  // with integer boundaries so every pixel belongs to exactly one target.
  if (item_width == 0) {
    int16_t x = 0;
    for (int i = 0; i < count; ++i) {
      const int16_t next_x = static_cast<int16_t>(
          (static_cast<int32_t>(i + 1) * available_width) / count);
      destinations_[i]->measure(WidthSpec::Exactly(next_x - x),
                                HeightSpec::Exactly(bar_height));
      static_cast<Widget&>(*destinations_[i])
          .layout(Rect(x, 0, next_x - 1, bar_height - 1));
      x = next_x;
    }
    return;
  }

  for (int i = 0; i < count; ++i) {
    const int16_t x = start_x + i * item_width;
    destinations_[i]->measure(WidthSpec::Exactly(item_width),
                              HeightSpec::Exactly(bar_height));
    static_cast<Widget&>(*destinations_[i])
        .layout(Rect(x, 0, x + item_width - 1, bar_height - 1));
  }
}

bool NavigationBar::onKeyEvent(const KeyEvent& event) {
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
  return context().focus().moveFocusDirection(*this, direction);
}

void NavigationBar::updateSelectionFromDestination(
    NavigationBarDestination& destination) {
  const int index = indexOf(destination);
  if (index < 0 || !destination.isEnabled()) return;
  onDestinationInvoked(index);
  if (index == selected_index_) {
    onSelectedDestinationReselected(index);
  } else {
    setSelectedIndex(index);
  }
}

void NavigationBar::propagateLayoutToDestinations() {
  for (NavigationBarDestination* destination : destinations_) {
    destination->setLayoutFromBar(layout());
  }
}

int NavigationBar::indexOf(const NavigationBarDestination& destination) const {
  for (int i = 0; i < destinationCount(); ++i) {
    if (destinations_[i] == &destination) return i;
  }
  return -1;
}

}  // namespace material3
}  // namespace roo_windows
