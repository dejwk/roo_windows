#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {
namespace material3 {

/// Selects the compact or expanded persistent navigation-rail layout.
enum class NavigationRailLayout : uint8_t {
  kCollapsed,
  kExpanded,
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
class NavigationRailDestination : public BasicWidget {
 public:
  /// Creates a destination with non-owning label and icon references.
  explicit NavigationRailDestination(ApplicationContext& context,
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

  /// Returns whether the parent rail currently selects this destination.
  bool selected() const;

  /// Returns the layout mode supplied by the parent rail.
  NavigationRailLayout layout() const;

  /// Navigation destinations are clickable while enabled.
  bool isClickable() const override;

  /// Owns interaction-overlay compositing for the destination indicator pill.
  OverlayType getOverlayType() const override { return OVERLAY_CUSTOM; }

  ClickOverlayAnimation getClickOverlayAnimation() const override {
    return ClickOverlayAnimation::kFade;
  }

  bool useOverlayOnSelection() const override { return false; }

  ColorToken effectiveOverlayColorRole() const override;
  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  void onSingleTapUp(XDim x, YDim y) override;
  void onClicked() override;
  void notifyStateChanged(uint16_t state_diff) override;

  /// Returns the content bounds that determine active-indicator geometry.
  virtual Rect destinationContentBounds() const;

  /// Returns the resolved icon slot used to anchor compact badges.
  Rect iconBounds() const;

  /// Returns the resolved label slot used for expanded badge anchoring.
  Rect labelBounds() const;

  Rect getDirectPaintExclusionBounds() const override;

 private:
  friend class NavigationRail;
  friend class NavigationRailDestinationTestAccess;

  void setLayoutFromRail(NavigationRailLayout layout);
  virtual void setLayoutDirectionFromRail(LayoutDirection direction) {}
  void setSelectedFromRail(bool selected);

  roo::string_view label_;
  const MonoIcon* icon_;
  const MonoIcon* selected_icon_;
  uint8_t layout_ : 1;
  uint8_t selected_ : 1;
  uint8_t click_handled_on_release_ : 1;
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
  uint8_t layout_direction_ : 1;
};

/// Material 3 persistent navigation-rail container with up to seven routes.
class NavigationRail : public Container {
 public:
  /// Maximum number of destinations supported by one navigation rail.
  static constexpr uint8_t kMaxDestinations = 7;

  /// Creates an empty rail in the collapsed, top-aligned layout.
  explicit NavigationRail(ApplicationContext& context);

  /// Detaches its header and destinations before destruction.
  ~NavigationRail() override;

  /// Returns the configured rail layout mode.
  NavigationRailLayout layout() const;

  /// Changes between collapsed and expanded rail layouts.
  void setLayout(NavigationRailLayout layout);

  /// Returns the destination-group alignment.
  NavigationRailGroupAlignment groupAlignment() const;

  /// Changes whether destinations are top- or center-aligned below the header.
  void setGroupAlignment(NavigationRailGroupAlignment alignment);

  /// Returns the logical direction used for badge placement.
  LayoutDirection layoutDirection() const;

  /// Changes the logical direction used for badge placement.
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

  ColorToken containerRole() const override;
  Color background() const override;
  void paint(PaintContext& ctx) const override;

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
  uint8_t layout_ : 1;
  uint8_t group_alignment_ : 1;
  uint8_t layout_direction_ : 1;
};

}  // namespace material3
}  // namespace roo_windows
