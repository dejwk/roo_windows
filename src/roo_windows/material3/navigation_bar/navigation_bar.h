#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/container.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {
namespace material3 {

/// Selects the compact vertical or medium horizontal destination layout.
enum class NavigationBarLayout : uint8_t {
  kVertical,
  kHorizontal,
};

class NavigationBar;
class NavigationBarDestinationTestAccess;

/// Badge-free Material 3 navigation-bar destination.
///
/// The parent `NavigationBar` owns selection and layout mode. The destination
/// stores only its label, icon references, and compact derived state so badge
/// storage remains opt-in through `BadgedNavigationBarDestination`.
class NavigationBarDestination : public BasicWidget {
 public:
  /// Creates a destination with non-owning label and icon references.
  explicit NavigationBarDestination(ApplicationContext& context,
                                    roo::string_view label = {},
                                    const MonoIcon* icon = nullptr,
                                    const MonoIcon* selected_icon = nullptr);

  /// Returns the current non-owning label view.
  roo::string_view label() const;

  /// Replaces the non-owning label view.
  void setLabel(roo::string_view label);

  /// Returns the inactive icon, or nullptr when none is configured.
  const MonoIcon* icon() const;

  /// Replaces the inactive icon reference.
  void setIcon(const MonoIcon* icon);

  /// Returns the selected icon, or nullptr when the inactive icon is reused.
  const MonoIcon* selectedIcon() const;

  /// Replaces the optional selected icon reference.
  void setSelectedIcon(const MonoIcon* icon);

  /// Returns whether the parent bar currently selects this destination.
  bool selected() const;

  /// Returns the layout mode supplied by the parent bar.
  NavigationBarLayout layout() const;

  /// Navigation destinations are clickable while enabled.
  bool isClickable() const override;

  /// Owns interaction-overlay compositing for the destination indicator pill.
  OverlayType getOverlayType() const override { return OVERLAY_CUSTOM; }

  ClickOverlayAnimation getClickOverlayAnimation() const override {
    return ClickOverlayAnimation::kFade;
  }

  bool useOverlayOnSelection() const override { return false; }

  ::roo_windows::material3::ColorToken effectiveOverlayColorRole()
      const override;

  /// Returns the token-backed compact or horizontal destination minimum.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Paints the always-visible label, selected icon, and active indicator.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Commits touch selection before the pill click animation reaches its
  /// settled frame.
  void onSingleTapUp(XDim x, YDim y) override;

  /// Routes keyboard and deferred framework activation through the bar after
  /// touch release has already committed selection when applicable.
  void onClicked() override;

  /// Repaints the indicator pill when interaction state changes.
  void notifyStateChanged(uint16_t state_diff) override;

  /// Returns the icon rectangle resolved by the destination layout.
  Rect iconBounds() const;

  /// Lets `paint()` register its disjoint settled content regions precisely.
  Rect getDirectPaintExclusionBounds() const override;

 private:
  friend class NavigationBar;
  friend class NavigationBarDestinationTestAccess;

  void setLayoutFromBar(NavigationBarLayout layout);
  void setSelectedFromBar(bool selected);

  roo::string_view label_;
  const MonoIcon* icon_;
  const MonoIcon* selected_icon_;
  uint8_t layout_ : 1;
  uint8_t selected_ : 1;
  uint8_t click_handled_on_release_ : 1;
};

/// Navigation-bar destination that opts into one inline badge helper.
class BadgedNavigationBarDestination : public NavigationBarDestination {
 public:
  /// Creates a badge-capable destination with non-owning label and icons.
  explicit BadgedNavigationBarDestination(
      ApplicationContext& context, roo::string_view label = {},
      const MonoIcon* icon = nullptr, const MonoIcon* selected_icon = nullptr);

  /// Returns the inline badge helper.
  const Badge& badge() const;

  /// Hides the badge.
  void hideBadge();

  /// Shows a dot badge.
  void setBadgeDot();

  /// Shows a text badge using the helper's fixed inline storage.
  void setBadgeText(roo::string_view text);

  /// Shows a decimal badge value, capped by the shared helper.
  void setBadgeValue(unsigned int number);

  /// Paints the badge before lower-z destination content.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Updates cached badge geometry after the destination is laid out.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  void relayoutBadge();

  Badge badge_;
};

/// Material 3 bottom navigation container with up to five destinations.
class NavigationBar : public Container {
 public:
  /// Maximum number of destinations supported by one navigation bar.
  static constexpr uint8_t kMaxDestinations = 5;

  /// Creates an empty navigation bar in the compact vertical layout.
  explicit NavigationBar(ApplicationContext& context);

  /// Detaches all borrowed or owned destinations before destruction.
  ~NavigationBar() override;

  /// Returns the configured destination layout mode.
  NavigationBarLayout layout() const;

  /// Changes the destination layout mode.
  void setLayout(NavigationBarLayout layout);

  /// Returns the selected destination index, or -1 when the bar is empty.
  int selectedIndex() const;

  /// Selects an existing destination by index.
  void setSelectedIndex(int index);

  /// Returns the current number of destinations.
  int destinationCount() const;

  /// Adds a borrowed or adopted destination when capacity permits.
  ///
  /// `destination` must reference a `NavigationBarDestination`. Pass
  /// `WidgetRef(destination)` to borrow a caller-owned destination or
  /// `WidgetRef(std::move(destination))` to transfer ownership.
  bool add(WidgetRef destination);

  /// Detaches every destination and clears selection.
  void clear();

  /// Resolves the navigation bar's Material surface token.
  ::roo_windows::material3::ColorToken containerRole() const override;

  /// Returns the Material 3 surface color owned by the bar container.
  Color background() const override;

  /// Paints the bar-owned surface once the container implementation lands.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Returns the number of destination children.
  int getChildrenCount() const override;

  /// Returns a destination child by index.
  const Widget& getChild(int idx) const override;

  /// Returns a mutable destination child by index.
  Widget& getChild(int idx) override;

  /// Measures the bar and destinations for the current layout mode.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Lays out destinations in their compact or horizontal arrangement.
  void onLayout(bool changed, const Rect& rect) override;

  /// Moves focus between destinations without changing selection.
  bool onKeyEvent(const KeyEvent& event) override;

  /// Called whenever an enabled destination is invoked.
  virtual void onDestinationInvoked(int index) {}

  /// Called after selection changes.
  virtual void onSelectedIndexChanged(int old_index, int new_index) {}

  /// Called when the selected destination is invoked again.
  virtual void onSelectedDestinationReselected(int index) {}

 private:
  friend class NavigationBarDestination;

  void updateSelectionFromDestination(NavigationBarDestination& destination);
  void propagateLayoutToDestinations();
  int indexOf(const NavigationBarDestination& destination) const;

  std::vector<NavigationBarDestination*> destinations_;
  int8_t selected_index_;
  uint8_t layout_ : 1;
};

}  // namespace material3
}  // namespace roo_windows
