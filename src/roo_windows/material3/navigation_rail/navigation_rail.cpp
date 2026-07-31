#include "roo_windows/material3/navigation_rail/navigation_rail.h"

#include <algorithm>

#include "roo_display/ui/text_label.h"
#include "roo_windows/material3/navigation_rail/navigation_rail_tokens.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows {
namespace material3 {
namespace {

const TextStyle& DestinationLabelStyle() { return text_style_label_medium(); }

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

Rect ExpandedIndicatorBounds(const Rect& target, const Rect& content_bounds) {
  if (target.empty() || content_bounds.empty()) return EmptyRect();
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  const int16_t indicator_width = std::min<int16_t>(
      target.width(), content_bounds.width() +
                          2 * Scaled(tokens.expanded_indicator_padding_dp));
  const int16_t indicator_height = std::min<int16_t>(
      target.height(), Scaled(tokens.expanded_indicator_height_dp));
  return CenteredBounds(
      Rect(content_bounds.xMin() - Scaled(tokens.expanded_indicator_padding_dp),
           target.yMin(),
           content_bounds.xMax() + Scaled(tokens.expanded_indicator_padding_dp),
           target.yMax()),
      indicator_width, indicator_height);
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
    result.label_width = DestinationLabelStyle()
                             .font()
                             .getHorizontalStringMetrics(
                                 label, DestinationLabelStyle().fontOptions())
                             .advance();
    result.label_height = DestinationLabelStyle().lineHeight();
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
    result.icon_bounds = CenteredBounds(
        result.indicator_bounds, metrics.icon_width, metrics.icon_height);
    if (metrics.label_height > 0) {
      const int16_t label_top =
          result.indicator_bounds.yMax() + 1 + Scaled(tokens.icon_label_gap_dp);
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
  const int16_t content_width = metrics.icon_width + gap + metrics.label_width;
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

  result.indicator_bounds =
      ExpandedIndicatorBounds(target, result.content_bounds);
  return result;
}

}  // namespace

NavigationRailDestination::NavigationRailDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : NavigationDestinationBase(context, label, icon, selected_icon) {}

NavigationRailLayout NavigationRailDestination::layout() const {
  return presentation() == internal::NavigationDestinationPresentation::kInline
             ? NavigationRailLayout::kExpanded
             : NavigationRailLayout::kCollapsed;
}

Dimensions NavigationRailDestination::getSuggestedMinimumDimensions() const {
  const MonoIcon* content_icon = displayedIcon();
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

internal::NavigationDestinationGeometry
NavigationRailDestination::resolveDestinationGeometry() const {
  DestinationContentGeometry geometry = ResolveContentGeometry(*this);
  if (layout() == NavigationRailLayout::kExpanded) {
    // Badged destinations extend this virtual content envelope with their
    // cached badge bounds so the active indicator hugs the full cluster.
    geometry.indicator_bounds =
        ExpandedIndicatorBounds(bounds(), destinationContentBounds());
  }
  return {geometry.icon_bounds, geometry.label_bounds,
          geometry.indicator_bounds};
}

Rect NavigationRailDestination::destinationContentBounds() const {
  return ResolveContentGeometry(*this).content_bounds;
}

Rect NavigationRailDestination::iconBounds() const {
  return ResolveContentGeometry(*this).icon_bounds;
}

Rect NavigationRailDestination::labelBounds() const {
  return ResolveContentGeometry(*this).label_bounds;
}

void NavigationRailDestination::setLayoutFromRail(NavigationRailLayout layout) {
  setPresentation(layout == NavigationRailLayout::kExpanded
                      ? internal::NavigationDestinationPresentation::kInline
                      : internal::NavigationDestinationPresentation::kStacked);
}

void NavigationRailDestination::setSelectedFromRail(bool selected) {
  setSelectedFromOwner(selected);
}

void NavigationRailDestination::activateFromOwner() {
  static_cast<NavigationRail*>(parent())->updateSelectionFromDestination(*this);
}

NavigationRail::NavigationRail(ApplicationContext& context)
    : Container(context),
      header_(nullptr),
      destinations_(),
      selected_index_(-1),
      layout_(static_cast<uint8_t>(NavigationRailLayout::kCollapsed)),
      group_alignment_(
          static_cast<uint8_t>(NavigationRailGroupAlignment::kTop)),
      layout_direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)) {}

NavigationRail::~NavigationRail() {
  clear();
  clearHeader();
}

NavigationRailLayout NavigationRail::layout() const {
  return static_cast<NavigationRailLayout>(layout_);
}

void NavigationRail::setLayout(NavigationRailLayout layout) {
  const uint8_t encoded = static_cast<uint8_t>(layout);
  if (layout_ == encoded) return;
  layout_ = encoded;
  propagateLayoutToDestinations();
  invalidateInterior();
  requestLayout();
}

NavigationRailGroupAlignment NavigationRail::groupAlignment() const {
  return static_cast<NavigationRailGroupAlignment>(group_alignment_);
}

void NavigationRail::setGroupAlignment(NavigationRailGroupAlignment alignment) {
  const uint8_t encoded = static_cast<uint8_t>(alignment);
  if (group_alignment_ == encoded) return;
  group_alignment_ = encoded;
  requestLayout();
}

LayoutDirection NavigationRail::layoutDirection() const {
  return static_cast<LayoutDirection>(layout_direction_);
}

void NavigationRail::setLayoutDirection(LayoutDirection direction) {
  const uint8_t encoded = static_cast<uint8_t>(direction);
  if (layout_direction_ == encoded) return;
  layout_direction_ = encoded;
  propagateLayoutToDestinations();
  invalidateInterior();
  requestLayout();
}

void NavigationRail::setHeader(WidgetRef header) {
  Widget* incoming = header.get();
  if (incoming == header_) return;
  if (incoming != nullptr) CHECK(incoming->parent() == nullptr);

  Widget* previous = header_;
  header_ = nullptr;
  if (previous != nullptr) detachChild(previous);
  if (incoming != nullptr) {
    header_ = incoming;
    attachChild(std::move(header));
  }
  invalidateInterior();
  requestLayout();
}

void NavigationRail::clearHeader() { setHeader(WidgetRef()); }

int NavigationRail::selectedIndex() const { return selected_index_; }

void NavigationRail::setSelectedIndex(int index) {
  if (index < 0 || index >= destinationCount() || index == selected_index_) {
    return;
  }
  const int old_index = selected_index_;
  selected_index_ = static_cast<int8_t>(index);
  for (int i = 0; i < destinationCount(); ++i) {
    destinations_[i]->setSelectedFromRail(i == selected_index_);
  }
  onSelectedIndexChanged(old_index, selected_index_);
  invalidateInterior();
}

int NavigationRail::destinationCount() const {
  return static_cast<int>(destinations_.size());
}

bool NavigationRail::add(WidgetRef destination) {
  if (destination.get() == nullptr || destinationCount() >= kMaxDestinations) {
    return false;
  }
  NavigationRailDestination* raw_destination =
      static_cast<NavigationRailDestination*>(destination.get());
  CHECK(raw_destination->parent() == nullptr);
  destinations_.push_back(raw_destination);
  attachChild(std::move(destination));
  raw_destination->setLayoutFromRail(layout());
  raw_destination->setLayoutDirectionFromRail(layoutDirection());
  if (selected_index_ < 0) {
    selected_index_ = 0;
    raw_destination->setSelectedFromRail(true);
  }
  invalidateInterior();
  requestLayout();
  return true;
}

void NavigationRail::clear() {
  if (destinations_.empty()) return;
  while (!destinations_.empty()) {
    NavigationRailDestination* destination = destinations_.back();
    destinations_.pop_back();
    detachChild(destination);
  }
  selected_index_ = -1;
  invalidateInterior();
  requestLayout();
}

::roo_windows::material3::ColorToken NavigationRail::containerRole() const {
  return ColorToken::kSurface;
}

Color NavigationRail::background() const {
  return theme().material3Theme().color.surface;
}

void NavigationRail::paint(PaintContext& ctx) const { ctx.clear(); }

int NavigationRail::getChildrenCount() const {
  return destinationCount() + (header_ != nullptr ? 1 : 0);
}

const Widget& NavigationRail::getChild(int idx) const {
  CHECK(idx >= 0);
  CHECK_LT(idx, getChildrenCount());
  if (header_ != nullptr) {
    if (idx == 0) return *header_;
    --idx;
  }
  return *destinations_[idx];
}

Widget& NavigationRail::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const NavigationRail&>(*this).getChild(idx));
}

Dimensions NavigationRail::onMeasure(WidthSpec width, HeightSpec height) {
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  // A rail has one token-defined width for each presentation. The parent may
  // still constrain that width through its WidthSpec, for example when a
  // scaffold must make room for body content.
  const XDim rail_width =
      width.resolveSize(Scaled(layout() == NavigationRailLayout::kCollapsed
                                   ? tokens.collapsed_min_width_dp
                                   : tokens.expanded_min_width_dp));
  // Children use the padded content width rather than the surface width. This
  // makes every destination's target area consistent with the eventual layout
  // and lets a generic header retain its natural width.
  const XDim content_width = std::max<XDim>(
      0, rail_width - 2 * Scaled(tokens.outer_horizontal_padding_dp));
  const YDim vertical_padding = Scaled(tokens.outer_vertical_padding_dp);
  YDim desired_height = 2 * vertical_padding;

  // The header is optional and is measured independently: unlike destinations
  // it is a caller-supplied composite and may choose a narrower natural width.
  // Its separation gap exists only when there is a destination group below it.
  if (header_ != nullptr && !header_->isGone()) {
    const Dimensions header_size = header_->measure(
        WidthSpec::AtMost(content_width), HeightSpec::Unspecified(0));
    desired_height += header_size.height();
    if (!destinations_.empty()) {
      desired_height += Scaled(tokens.header_destination_gap_dp);
    }
  }

  // Destinations always receive the full content width and stack vertically.
  // Height remains unconstrained here so each destination can report its
  // layout-mode minimum; the parent HeightSpec resolves the final rail height.
  for (int i = 0; i < destinationCount(); ++i) {
    const Dimensions destination_size = destinations_[i]->measure(
        WidthSpec::Exactly(content_width), HeightSpec::Unspecified(0));
    desired_height += destination_size.height();
    if (i > 0) desired_height += Scaled(tokens.destination_gap_dp);
  }
  return Dimensions(rail_width, height.resolveSize(desired_height));
}

void NavigationRail::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const internal::NavigationRailTokens& tokens =
      internal::kNavigationRailTokens;
  if (rect.empty()) {
    // An empty parent target must also clear stale child geometry. Otherwise a
    // detached or clipped rail could leave old child hit targets reachable.
    if (header_ != nullptr) static_cast<Widget&>(*header_).layout(EmptyRect());
    for (NavigationRailDestination* destination : destinations_) {
      static_cast<Widget&>(*destination).layout(EmptyRect());
    }
    return;
  }

  const XDim horizontal_padding = Scaled(tokens.outer_horizontal_padding_dp);
  const YDim vertical_padding = Scaled(tokens.outer_vertical_padding_dp);
  // The rail surface owns the outer padding. Destinations receive this entire
  // inner width, which is the full-width target area described by the rail
  // contract; their indicator remains narrower and is resolved internally.
  const Rect content(
      rect.xMin() + horizontal_padding, rect.yMin() + vertical_padding,
      rect.xMax() - horizontal_padding, rect.yMax() - vertical_padding);
  if (content.empty()) {
    if (header_ != nullptr) static_cast<Widget&>(*header_).layout(EmptyRect());
    for (NavigationRailDestination* destination : destinations_) {
      static_cast<Widget&>(*destination).layout(EmptyRect());
    }
    return;
  }

  YDim destination_top = content.yMin();
  if (header_ != nullptr && !header_->isGone()) {
    // Keep the generic header at its natural measured size and center it in
    // the rail. The destination group then occupies only the remaining band.
    const Dimensions header_size =
        header_->measure(WidthSpec::AtMost(content.width()),
                         HeightSpec::AtMost(content.height()));
    const XDim header_width =
        std::min<XDim>(content.width(), header_size.width());
    const YDim header_height =
        std::min<YDim>(content.height(), header_size.height());
    const XDim header_left =
        content.xMin() + (content.width() - header_width) / 2;
    static_cast<Widget&>(*header_).layout(
        Rect(header_left, content.yMin(), header_left + header_width - 1,
             content.yMin() + header_height - 1));
    destination_top = content.yMin() + header_height;
    if (!destinations_.empty()) {
      destination_top = std::min<YDim>(
          content.yMax() + 1,
          destination_top + Scaled(tokens.header_destination_gap_dp));
    }
  } else if (header_ != nullptr) {
    static_cast<Widget&>(*header_).layout(EmptyRect());
  }

  if (destinations_.empty() || destination_top > content.yMax()) return;
  const YDim available_height = content.yMax() - destination_top + 1;
  const int count = destinationCount();
  const YDim minimum_height = Scaled(tokens.destination_height_dp);
  const YDim preferred_gaps = Scaled(tokens.destination_gap_dp);
  const int32_t preferred_total =
      static_cast<int32_t>(count) * minimum_height +
      static_cast<int32_t>(count - 1) * preferred_gaps;
  // Preserve token-height destinations whenever possible. Under pressure the
  // inter-item gaps are the first discretionary space to disappear.
  const YDim gap = preferred_total <= available_height ? preferred_gaps : 0;
  const YDim group_height = preferred_total <= available_height
                                ? static_cast<YDim>(preferred_total)
                                : available_height;
  if (groupAlignment() == NavigationRailGroupAlignment::kCenter &&
      group_height < available_height) {
    // Center only the destination group; the header stays top-aligned.
    destination_top += (available_height - group_height) / 2;
  }

  if (preferred_total <= available_height) {
    for (int i = 0; i < count; ++i) {
      NavigationRailDestination* destination = destinations_[i];
      destination->measure(WidthSpec::Exactly(content.width()),
                           HeightSpec::Exactly(minimum_height));
      static_cast<Widget&>(*destination)
          .layout(Rect(content.xMin(), destination_top, content.xMax(),
                       destination_top + minimum_height - 1));
      destination_top += minimum_height + gap;
    }
    return;
  }

  // Under vertical pressure, free gaps are removed before destinations shrink.
  // Integer boundaries guarantee that every available pixel belongs to one
  // target, preserving the full-width hit-test contract.
  for (int i = 0; i < count; ++i) {
    const YDim next_top = static_cast<YDim>(
        destination_top +
        (static_cast<int32_t>(i + 1) * available_height) / count);
    NavigationRailDestination* destination = destinations_[i];
    destination->measure(WidthSpec::Exactly(content.width()),
                         HeightSpec::Exactly(next_top - destination_top));
    static_cast<Widget&>(*destination)
        .layout(Rect(content.xMin(), destination_top, content.xMax(),
                     next_top - 1));
    destination_top = next_top;
  }
}

bool NavigationRail::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat) {
    return false;
  }
  FocusDirection direction;
  switch (event.code) {
    case KeyCode::kUp:
      direction = FocusDirection::kUp;
      break;
    case KeyCode::kDown:
      direction = FocusDirection::kDown;
      break;
    default:
      return false;
  }
  return context().focus().moveFocusDirection(*this, direction);
}

void NavigationRail::updateSelectionFromDestination(
    NavigationRailDestination& destination) {
  const int index = indexOf(destination);
  if (index < 0 || !destination.isEnabled()) return;
  onDestinationInvoked(index);
  if (index == selected_index_) {
    onSelectedDestinationReselected(index);
  } else {
    setSelectedIndex(index);
  }
}

void NavigationRail::propagateLayoutToDestinations() {
  for (NavigationRailDestination* destination : destinations_) {
    destination->setLayoutFromRail(layout());
    destination->setLayoutDirectionFromRail(layoutDirection());
  }
}

int NavigationRail::indexOf(
    const NavigationRailDestination& destination) const {
  for (int i = 0; i < destinationCount(); ++i) {
    if (destinations_[i] == &destination) return i;
  }
  return -1;
}

BadgedNavigationRailDestination::BadgedNavigationRailDestination(
    ApplicationContext& context, roo::string_view label, const MonoIcon* icon,
    const MonoIcon* selected_icon)
    : NavigationRailDestination(context, label, icon, selected_icon),
      badge_(),
      layout_direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)) {}

const Badge& BadgedNavigationRailDestination::badge() const { return badge_; }

void BadgedNavigationRailDestination::hideBadge() {
  if (!badge_.visible()) return;
  badge_.hide();
  invalidateInterior();
}

void BadgedNavigationRailDestination::setBadgeDot() {
  badge_.setDot();
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationRailDestination::setBadgeText(roo::string_view text) {
  badge_.setText(text);
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationRailDestination::setBadgeValue(unsigned int number) {
  badge_.setValue(number);
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationRailDestination::paint(PaintContext& ctx) const {
  // The badge is front-most. Its helper settles its direct pixels and records
  // exclusions before base paint fills the lower-z indicator and rail surface.
  badge_.paint(ctx, theme());
  NavigationRailDestination::paint(ctx);
}

void BadgedNavigationRailDestination::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  relayoutBadge();
}

Rect BadgedNavigationRailDestination::destinationContentBounds() const {
  Rect content = NavigationRailDestination::destinationContentBounds();
  if (layout() == NavigationRailLayout::kExpanded && badge_.visible()) {
    content = UnionBounds(content, badge_.bounds());
  }
  return content;
}

void BadgedNavigationRailDestination::setLayoutDirectionFromRail(
    LayoutDirection direction) {
  const uint8_t encoded = static_cast<uint8_t>(direction);
  if (layout_direction_ == encoded) return;
  layout_direction_ = encoded;
  relayoutBadge();
  invalidateInterior();
}

void BadgedNavigationRailDestination::relayoutBadge() {
  if (!badge_.visible()) return;
  if (layout() == NavigationRailLayout::kCollapsed) {
    badge_.layoutForIcon(iconBounds());
    return;
  }
  const Rect anchor = badgeAnchorBounds();
  if (anchor.empty()) return;

  const bool rtl = static_cast<LayoutDirection>(layout_direction_) ==
                   LayoutDirection::kRightToLeft;
  roo_display::Alignment alignment;
  // A one-pixel synthetic anchor just beyond the label maps the shared
  // corner-based helper onto the Material expanded beside-label treatment.
  alignment = rtl ? roo_display::kRight | roo_display::kTop
                  : roo_display::kLeft | roo_display::kTop;
  if (!badge_.layout(anchor, alignment)) return;

  const Rect badge_bounds = badge_.bounds();
  int16_t horizontal_delta = 0;
  int16_t vertical_delta = 0;
  if (badge_bounds.xMin() < bounds().xMin()) {
    horizontal_delta = bounds().xMin() - badge_bounds.xMin();
  } else if (badge_bounds.xMax() > bounds().xMax()) {
    horizontal_delta = bounds().xMax() - badge_bounds.xMax();
  }
  if (badge_bounds.yMin() < bounds().yMin()) {
    vertical_delta = bounds().yMin() - badge_bounds.yMin();
  } else if (badge_bounds.yMax() > bounds().yMax()) {
    vertical_delta = bounds().yMax() - badge_bounds.yMax();
  }
  if (horizontal_delta == 0 && vertical_delta == 0) return;

  badge_.layout(anchor, alignment.shiftBy(horizontal_delta, vertical_delta));
}

Rect BadgedNavigationRailDestination::badgeAnchorBounds() const {
  if (layout() == NavigationRailLayout::kCollapsed) return iconBounds();

  Rect label_bounds = labelBounds();
  if (label_bounds.empty()) label_bounds = iconBounds();
  if (label_bounds.empty()) return EmptyRect();

  const bool rtl = static_cast<LayoutDirection>(layout_direction_) ==
                   LayoutDirection::kRightToLeft;
  const int16_t anchor_x =
      rtl ? label_bounds.xMin() - 1 : label_bounds.xMax() + 1;
  return Rect(anchor_x, label_bounds.yMin(), anchor_x, label_bounds.yMax());
}

}  // namespace material3
}  // namespace roo_windows
