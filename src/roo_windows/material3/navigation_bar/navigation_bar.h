#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "roo_backport/string_view.h"
#include "roo_windows/core/container.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_destination/navigation_destination.h"
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
class NavigationBarDestination : public internal::NavigationDestinationBase {
 public:
  /// Creates a destination with non-owning label and icon references.
  explicit NavigationBarDestination(ApplicationContext& context,
                                    roo::string_view label = {},
                                    const MonoIcon* icon = nullptr,
                                    const MonoIcon* selected_icon = nullptr);

  /// Returns the layout mode supplied by the parent bar.
  NavigationBarLayout layout() const;

  /// Returns the token-backed compact or horizontal destination minimum.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  /// Returns the icon rectangle resolved by the destination layout.
  Rect iconBounds() const;

 private:
  friend class NavigationBar;
  friend class NavigationBarDestinationTestAccess;

  void setLayoutFromBar(NavigationBarLayout layout);
  void setSelectedFromBar(bool selected);
  internal::NavigationDestinationGeometry resolveDestinationGeometry()
      const override;
  void activateFromOwner() override;
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
