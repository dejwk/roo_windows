#pragma once

#include <stdint.h>

#include "roo_display/color/color.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/insets.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/rect.h"

namespace roo_windows::material3 {

/// Names the five width classes used by Material 3 adaptive layouts.
enum class LayoutBreakpoint : uint8_t {
  kCompact,
  kMedium,
  kExpanded,
  kLarge,
  kExtraLarge,
};

/// Inclusive breakpoint interval used by future scaffold slots.
struct BreakpointRange {
  LayoutBreakpoint min = LayoutBreakpoint::kCompact;
  LayoutBreakpoint max = LayoutBreakpoint::kExtraLarge;

  /// Returns whether `breakpoint` belongs to this inclusive interval.
  bool contains(LayoutBreakpoint breakpoint) const;
};

/// Material ruler tokens authored in density-independent pixels.
struct BreakpointTokens {
  uint8_t columns;
  int16_t outer_margin_dp;
  int16_t gutter_dp;
};

/// Width-class lower bounds authored in density-independent pixels.
struct BreakpointThresholds {
  int16_t medium_min_dp = 600;
  int16_t expanded_min_dp = 840;
  int16_t large_min_dp = 1200;
  int16_t extra_large_min_dp = 1600;
};

/// Resolved ruler geometry in the owner's local pixel coordinate space.
struct LayoutMetrics {
  LayoutBreakpoint breakpoint = LayoutBreakpoint::kCompact;
  LayoutDirection direction = LayoutDirection::kLeftToRight;
  Rect safe_bounds = Rect(0, 0, -1, -1);
  Rect content_bounds = Rect(0, 0, -1, -1);
  uint8_t columns = 1;
  int16_t outer_margin = 0;
  int16_t gutter = 0;
  int16_t column_width = 0;

  /// Returns the physical left edge of a logical column, or zero if invalid.
  XDim columnStart(uint8_t column) const;

  /// Returns a logical column span, mirrored physically for RTL layouts.
  Rect spanBounds(uint8_t first_column, uint8_t span, YDim top,
                  YDim height) const;
};

/// Immutable width-breakpoint and ruler-token policy.
class LayoutBreakpointPolicy {
 public:
  /// Creates a validated policy using Material 3-compatible defaults.
  LayoutBreakpointPolicy(
      BreakpointThresholds thresholds = BreakpointThresholds(),
      BreakpointTokens compact = {4, 16, 16},
      BreakpointTokens medium = {8, 24, 16},
      BreakpointTokens expanded = {12, 24, 24},
      BreakpointTokens large = {12, 32, 24},
      BreakpointTokens extra_large = {12, 40, 24});

  /// Returns the process-lifetime default policy.
  static const LayoutBreakpointPolicy& Default();

  /// Resolves a pixel width against this policy's scaled dp thresholds.
  LayoutBreakpoint resolveWidthPx(XDim width_px) const;

  /// Returns the tokens selected by a resolved breakpoint.
  const BreakpointTokens& tokens(LayoutBreakpoint breakpoint) const;

  /// Resolves an allocation-free ruler for a local safe pixel rectangle.
  ///
  /// Tiny rectangles retain at least one content pixel whenever possible by
  /// clamping margin first, then reducing columns until the gutters fit.
  LayoutMetrics resolveMetrics(
      const Rect& safe_bounds,
      LayoutDirection direction = LayoutDirection::kLeftToRight) const;

  /// Resolves a ruler with an explicitly selected outer breakpoint.
  ///
  /// Scaffolds use this after chrome has narrowed their body band, while
  /// nested grids normally use `resolveMetrics()` and classify locally.
  LayoutMetrics resolveMetricsForBreakpoint(
      const Rect& safe_bounds, LayoutBreakpoint breakpoint,
      LayoutDirection direction = LayoutDirection::kLeftToRight) const;

 private:
  BreakpointThresholds thresholds_;
  BreakpointTokens tokens_[5];
};

/// Fixed-slot Material 3 application shell for top-level page chrome.
class LayoutScaffold : public Container {
 public:
  /// Creates an empty match-parent scaffold using the default policy.
  explicit LayoutScaffold(ApplicationContext& context);

  /// Detaches all stored slots before their references are released.
  ~LayoutScaffold() override;

  /// Borrows an immutable breakpoint policy that must outlive this scaffold.
  void setBreakpointPolicy(const LayoutBreakpointPolicy& policy);
  void setBreakpointPolicy(LayoutBreakpointPolicy&&) = delete;

  /// Changes the logical leading/trailing direction and requests layout.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the explicit logical leading/trailing direction.
  LayoutDirection layoutDirection() const;

  /// Sets physical caller-supplied safety insets, clamping negative edges.
  void setSafetyInsets(Insets insets);

  /// Replaces the top-bar slot and its breakpoint participation rule.
  void setTopBar(WidgetRef widget,
                 BreakpointRange visibility = BreakpointRange());

  /// Changes the top-bar participation rule without replacing its child.
  void setTopBarVisibility(BreakpointRange visibility);

  /// Clears the top-bar slot.
  void clearTopBar();

  /// Replaces the bottom-bar slot and its breakpoint participation rule.
  void setBottomBar(WidgetRef widget,
                    BreakpointRange visibility = BreakpointRange());

  /// Changes the bottom-bar participation rule without replacing its child.
  void setBottomBarVisibility(BreakpointRange visibility);

  /// Clears the bottom-bar slot.
  void clearBottomBar();

  /// Replaces the logical leading rail, expanded and wider by default.
  void setLeadingRail(WidgetRef widget, BreakpointRange visibility = {
                                            LayoutBreakpoint::kExpanded,
                                            LayoutBreakpoint::kExtraLarge});

  /// Changes the leading-rail participation rule without replacing its child.
  void setLeadingRailVisibility(BreakpointRange visibility);

  /// Clears the leading-rail slot.
  void clearLeadingRail();

  /// Replaces the logical trailing rail and its breakpoint participation rule.
  void setTrailingRail(WidgetRef widget,
                       BreakpointRange visibility = BreakpointRange());

  /// Changes the trailing-rail participation rule without replacing its child.
  void setTrailingRailVisibility(BreakpointRange visibility);

  /// Clears the trailing-rail slot.
  void clearTrailingRail();

  /// Replaces the body slot; a null body publishes empty body geometry.
  void setBody(WidgetRef widget);

  /// Returns the latest resolved scaffold ruler metrics.
  const LayoutMetrics& metrics() const { return metrics_; }

  /// Returns the resolved body band, or an empty rectangle before layout.
  Rect bodyBounds() const { return metrics_.safe_bounds; }

  /// Returns the physical chrome and safety insets around `bodyBounds()`.
  Insets contentInsets() const { return content_insets_; }

  /// Returns the active bottom-bar rectangle, or an empty rectangle.
  Rect bottomBarBounds() const { return bottom_bar_bounds_; }

  /// Owns the Material page-background surface.
  roo_display::Color background() const override;

  /// Owns the Material page-background color role.
  ::roo_windows::material3::ColorToken containerRole() const override;

 protected:
  PreferredSize getPreferredSize() const override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  static Insets ClampInsets(Insets insets);
  static Rect ApplyInsets(const Rect& rect, Insets insets);
  static Rect EmptyRect();

  void replaceSlot(Widget*& slot, WidgetRef widget);
  void setSlotVisibility(BreakpointRange& target, BreakpointRange visibility);
  void updateChromeVisibility(LayoutBreakpoint breakpoint);
  void clearLayoutMetrics(LayoutBreakpoint breakpoint);
  void layoutSlot(Widget* widget, const Rect& bounds);
  Widget* top_bar_;
  Widget* bottom_bar_;
  Widget* leading_rail_;
  Widget* trailing_rail_;
  Widget* body_;
  const LayoutBreakpointPolicy* policy_;
  BreakpointRange top_bar_visibility_;
  BreakpointRange bottom_bar_visibility_;
  BreakpointRange leading_rail_visibility_;
  BreakpointRange trailing_rail_visibility_;
  Insets safety_insets_;
  Insets content_insets_;
  LayoutMetrics metrics_;
  Rect bottom_bar_bounds_;
  int16_t top_bar_height_;
  int16_t bottom_bar_height_;
  int16_t leading_rail_width_;
  int16_t trailing_rail_width_;
  uint8_t direction_ : 1;
};

static_assert(sizeof(LayoutScaffold) <=
                  sizeof(Container) + 6 * sizeof(void*) +
                      4 * sizeof(BreakpointRange) + 2 * sizeof(Insets) +
                      sizeof(LayoutMetrics) + sizeof(Rect) + 16,
              "LayoutScaffold must retain its fixed-slot RAM budget");

}  // namespace roo_windows::material3
