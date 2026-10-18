#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "roo_backport/string_view.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/container.h"
#include "roo_windows/material3/navigation_destination/navigation_destination.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {
namespace material3 {

/// Selects baseline, collapsed wide, or expanded persistent rail geometry.
enum class NavigationRailLayout : uint8_t {
  /// Material 3 collapsed wide rail: 96 dp wide with 64 dp vertical items.
  kCollapsed,
  /// Expanded rail with horizontal labels and 64 dp destination slots.
  kExpanded,
  /// Baseline 80 dp rail with stacked labels and compact header spacing.
  kBaseline,
};

/// Selects where the destination group sits below the optional header.
enum class NavigationRailGroupAlignment : uint8_t {
  kTop,
  kCenter,
};

class NavigationRail;
class NavigationRailDestinationTestAccess;

/// Badge-free Material 3 navigation-rail destination.
///
/// The parent `NavigationRail` owns selection and layout mode. The destination
/// stores only its label, icon references, and compact derived state so badge
/// storage remains opt-in through `BadgedNavigationRailDestination`.
class NavigationRailDestination : public internal::NavigationDestinationBase {
 public:
  /// Creates a destination with non-owning label and icon references.
  explicit NavigationRailDestination(ApplicationContext& context,
                                     roo::string_view label = {},
                                     const MonoIcon* icon = nullptr,
                                     const MonoIcon* selected_icon = nullptr);

  /// Activates on click confirmation while the feedback animation continues.
  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateContinueAnimation;
  }

  /// Returns the layout mode supplied by the parent rail.
  NavigationRailLayout layout() const;
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  /// Returns the content bounds that determine active-indicator geometry.
  virtual Rect destinationContentBounds() const;

  /// Returns the resolved icon slot used to anchor compact badges.
  Rect iconBounds() const;

  /// Returns the resolved label slot used for expanded badge anchoring.
  Rect labelBounds() const;

  /// Uses the rail label role outside the collapsed selection indicator.
  Color labelColor() const override;

  /// Uses label-large typography for horizontal destinations.
  const TextStyle& labelStyle() const override;

  /// Returns the logical direction supplied by the rail.
  LayoutDirection layoutDirection() const;

  /// Updates direction and lets badge hosts refresh their cached placement.
  virtual void setLayoutDirectionFromRail(LayoutDirection direction);

 private:
  friend class NavigationRail;
  friend class NavigationRailDestinationTestAccess;

  void setLayoutFromRail(NavigationRailLayout layout);
  void setSelectedFromRail(bool selected);
  internal::NavigationDestinationGeometry resolveDestinationGeometry()
      const override;
  void activateFromOwner() override;

  uint8_t layout_ : 2;
  uint8_t layout_direction_ : 1;
};

/// Navigation-rail destination that opts into one inline badge helper.
class BadgedNavigationRailDestination : public NavigationRailDestination {
 public:
  /// Creates a badge-capable destination with non-owning label and icons.
  explicit BadgedNavigationRailDestination(
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

  void paint(PaintContext& ctx) const override;

 protected:
  void onLayout(bool changed, const Rect& rect) override;
  Rect destinationContentBounds() const override;
  void setLayoutDirectionFromRail(LayoutDirection direction) override;

 private:
  void relayoutBadge();
  Rect badgeAnchorBounds() const;

  Badge badge_;
};

/// Material 3 persistent navigation-rail container with up to seven routes.
class NavigationRail : public Material3Container {
 public:
  /// Maximum number of destinations supported by one navigation rail.
  static constexpr uint8_t kMaxDestinations = 7;

  /// Creates an empty rail in the collapsed, top-aligned layout.
  explicit NavigationRail(ApplicationContext& context);

  /// Detaches its header and destinations before destruction.
  ~NavigationRail() override;

  /// Allows caller-configured header and destinations to be unclipped.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Returns the configured rail layout mode.
  NavigationRailLayout layout() const;

  /// Changes between baseline, collapsed wide, and expanded rail layouts.
  void setLayout(NavigationRailLayout layout);

  /// Returns the destination-group alignment.
  NavigationRailGroupAlignment groupAlignment() const;

  /// Changes whether destinations are top- or center-aligned below the header.
  void setGroupAlignment(NavigationRailGroupAlignment alignment);

  /// Returns the logical direction used for content and badge placement.
  LayoutDirection layoutDirection() const;

  /// Changes the logical direction used for content and badge placement.
  void setLayoutDirection(LayoutDirection direction);

  /// Replaces the optional header child with a borrowed or adopted widget.
  void setHeader(WidgetRef header);

  /// Detaches the optional header child.
  void clearHeader();

  /// Returns the selected destination index, or -1 when empty.
  int selectedIndex() const;

  /// Selects an existing destination by index.
  void setSelectedIndex(int index);

  /// Returns the current number of destinations.
  int destinationCount() const;

  /// Adds a borrowed or adopted destination when capacity permits.
  ///
  /// `destination` must reference a `NavigationRailDestination`. Pass
  /// `WidgetRef(destination)` to borrow a caller-owned destination or
  /// `WidgetRef(std::move(destination))` to transfer ownership.
  bool add(WidgetRef destination);

  /// Detaches every destination and clears selection, retaining the header.
  void clear();

  /// Returns the explicit surface role, or the layout-specific theme default.
  ColorToken containerRole() const override;

  /// Overrides the rail surface, e.g. kSurfaceContainer. kNone inherits the
  /// parent surface color; it does not introduce transparent painting.
  void setContainerRole(ColorToken role);

  /// Restores the layout-specific theme default surface role.
  void clearContainerRoleOverride();

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  bool onKeyEvent(const KeyEvent& event) override;

  /// Called whenever an enabled destination is invoked.
  virtual void onDestinationInvoked(int index) {}

  /// Called after selection changes.
  virtual void onSelectedIndexChanged(int old_index, int new_index) {}

  /// Called when the selected destination is invoked again.
  virtual void onSelectedDestinationReselected(int index) {}

 private:
  friend class NavigationRailDestination;

  void updateSelectionFromDestination(NavigationRailDestination& destination);
  void propagateLayoutToDestinations();
  int indexOf(const NavigationRailDestination& destination) const;

  Widget* header_;
  std::vector<NavigationRailDestination*> destinations_;
  int8_t selected_index_;
  ColorToken container_role_override_;
  uint8_t has_container_role_override_ : 1;
  uint8_t layout_ : 2;
  uint8_t group_alignment_ : 1;
  uint8_t layout_direction_ : 1;
};

}  // namespace material3
}  // namespace roo_windows
