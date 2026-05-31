#include "roo_windows/material3/switch/badged_switch.h"

#include <algorithm>

namespace roo_windows {
namespace material3 {

namespace {

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

Insets InsetsFromEnvelope(const Rect& logical_bounds, const Rect& envelope) {
  Rect combined = UnionRects(logical_bounds, envelope);
  return Insets(combined.xMin() - logical_bounds.xMin(),
                combined.yMin() - logical_bounds.yMin(),
                logical_bounds.xMax() - combined.xMax(),
                logical_bounds.yMax() - combined.yMax());
}

bool SamePlacement(BadgePlacement a, BadgePlacement b) {
  return a.gravity == b.gravity && a.horizontal_offset == b.horizontal_offset &&
         a.vertical_offset == b.vertical_offset;
}

}  // namespace

BadgedSwitch::BadgedSwitch(ApplicationContext& context, bool on)
    : Switch(context, on ? Switch::OnOffState::kOn : Switch::OnOffState::kOff),
      badge_(),
      badge_placement_() {
  setParentClipMode(ParentClipMode::kUnclipped);
}

void BadgedSwitch::setBadgePlacement(BadgePlacement placement) {
  if (SamePlacement(badge_placement_, placement)) return;
  Rect old_bounds = badge_.bounds();
  badge_placement_ = placement;
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedSwitch::hideBadge() {
  if (!badge_.visible()) return;
  Rect old_bounds = badge_.bounds();
  badge_.hide();
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedSwitch::setBadgeDot() {
  if (badge_.mode() == BadgeMode::kDot) return;
  Rect old_bounds = badge_.bounds();
  badge_.setDot();
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedSwitch::setBadgeText(roo::string_view text) {
  if (badge_.mode() == BadgeMode::kText && badge_.text() == text) return;
  Rect old_bounds = badge_.bounds();
  badge_.setText(text);
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

void BadgedSwitch::setBadgeValue(unsigned int number) {
  Rect old_bounds = badge_.bounds();
  badge_.setValue(number);
  Rect new_bounds = relayoutBadge();
  handleBadgeGeometryChange(old_bounds, new_bounds);
}

Insets BadgedSwitch::getInkInsets() const {
  if (!badge_.visible()) return Switch::getInkInsets();
  return InsetsFromEnvelope(bounds(), conservativeBadgeBounds());
}

void BadgedSwitch::paint(PaintContext& ctx) const { Switch::paint(ctx); }

void BadgedSwitch::paintWidgetContents(PaintContext& ctx) {
  badge_.paint(ctx, theme());
  Switch::paintWidgetContents(ctx);
}

void BadgedSwitch::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  const_cast<BadgedSwitch*>(this)->relayoutBadge();
}

Rect BadgedSwitch::getDirectPaintExclusionBounds() const { return bounds(); }

Rect BadgedSwitch::badgeAnchorBounds() const {
  Dimensions track_dims = Switch::getSuggestedMinimumDimensions();
  Rect logical_bounds = bounds();
  if (logical_bounds.empty() || track_dims.width() <= 0 ||
      track_dims.height() <= 0) {
    return EmptyRect();
  }
  int16_t left =
      logical_bounds.xMin() + (logical_bounds.width() - track_dims.width()) / 2;
  int16_t top = logical_bounds.yMin() +
                (logical_bounds.height() - track_dims.height()) / 2;
  return Rect(left, top, left + track_dims.width() - 1,
              top + track_dims.height() - 1);
}

Rect BadgedSwitch::conservativeBadgeBounds() const {
  if (!badge_.visible()) return EmptyRect();
  return Badge::ConservativeBounds(badgeAnchorBounds(), badge_placement_,
                                   badge_.mode() == BadgeMode::kText);
}

Rect BadgedSwitch::relayoutBadge() {
  if (!badge_.visible()) return EmptyRect();
  if (!badge_.layout(badgeAnchorBounds(), badge_placement_)) {
    return EmptyRect();
  }
  return badge_.bounds();
}

void BadgedSwitch::handleBadgeGeometryChange(const Rect& old_bounds,
                                             const Rect& new_bounds) {
  Rect repaint_bounds = UnionRects(old_bounds, new_bounds);
  if (repaint_bounds.empty()) return;
  setDirty(repaint_bounds);
  if (parent() != nullptr) {
    notifyParentInvalidatedRegion(
        repaint_bounds.translate(offsetLeft(), offsetTop()));
  }
  requestLayout();
}

}  // namespace material3
}  // namespace roo_windows