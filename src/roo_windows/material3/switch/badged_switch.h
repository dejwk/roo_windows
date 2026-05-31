#pragma once

#include "roo_backport/string_view.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/switch/switch.h"

namespace roo_windows {
namespace material3 {

/// Material 3 switch subclass with an inline badge adornment.
///
/// The base `Switch` remains badge-free; only this subclass pays for the
/// additional `Badge` state and the overflow/invalidation wiring needed for
/// badge overhang.
class BadgedSwitch : public Switch {
 public:
  /// Creates a Material 3 switch that can render a badge over its track.
  explicit BadgedSwitch(ApplicationContext& context, bool on = false);

  /// Returns the current badge state.
  const Badge& badge() const { return badge_; }

  /// Returns the current badge placement relative to the switch track.
  BadgePlacement badgePlacement() const { return badge_placement_; }

  /// Updates the badge placement and repaints the old/new badge envelope.
  void setBadgePlacement(BadgePlacement placement);

  /// Hides the badge and repaints the old badge envelope.
  void hideBadge();

  /// Shows a dot badge and repaints the old/new badge envelope.
  void setBadgeDot();

  /// Shows a text badge and repaints the old/new badge envelope.
  void setBadgeText(roo::string_view text);

  /// Shows a numeric badge and repaints the old/new badge envelope.
  void setBadgeValue(unsigned int number);

  // Widget overrides.
  /// Expands the switch ink bounds conservatively when the badge can overhang.
  Insets getInkInsets() const override;

  /// Paints only the base switch geometry; badge ordering is handled in
  /// `paintWidgetContents()`.
  void paint(PaintContext& ctx) const override;

  /// Settles the badge before switch-local lower-z content.
  void paintWidgetContents(PaintContext& ctx) override;

 protected:
  /// Re-resolves the badge layout when the host switch bounds change.
  void onLayout(bool changed, const Rect& rect) override;

  /// Leaves badge corner-strip repaint to the badge's own exclusion logic.
  Rect getDirectPaintExclusionBounds() const override;

 private:
  /// Returns the centered local track rect used as the badge anchor.
  Rect badgeAnchorBounds() const;

  /// Returns the conservative local badge overflow envelope.
  Rect conservativeBadgeBounds() const;

  /// Lays out the owned badge against the current anchor and returns bounds.
  Rect relayoutBadge();

  /// Repaints and invalidates the union of old/new badge envelopes.
  void handleBadgeGeometryChange(const Rect& old_bounds,
                                 const Rect& new_bounds);

  Badge badge_;
  BadgePlacement badge_placement_;
};

}  // namespace material3
}  // namespace roo_windows