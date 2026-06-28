#pragma once

#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "roo_backport/string_view.h"
#include "roo_scheduler.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {
namespace material3 {

enum class TabsVariant : uint8_t {
  kPrimary,
  kSecondary,
};

enum class TabsMode : uint8_t {
  kFixed,
  kScrollable,
};

enum class TabsSelectionCommitMode : uint8_t {
  kOnRelease,
  kAfterClickAnimation,
};

class Tabs;

/// Material 3 tab leaf with a single-line label and optional icon.
///
/// `Tab` stores label text as a non-owning view. Selection is owned by the
/// parent `Tabs` row and reflected through the widget activated state.
class Tab : public SurfaceWidget {
 public:
  /// Creates a tab with the supplied non-owning label view.
  explicit Tab(ApplicationContext& context, roo::string_view label = {});

  /// Returns the current non-owning label view.
  roo::string_view label() const { return label_; }

  /// Replaces the label view and requests repaint/layout.
  void setLabel(roo::string_view label);

  /// Returns whether this tab has an icon.
  bool hasIcon() const { return icon_ != nullptr; }

  /// Returns the configured icon, or nullptr when unset.
  const MonoIcon* icon() const { return icon_; }

  /// Sets or clears the optional icon. The tab does not take ownership.
  void setIcon(const MonoIcon* icon);

  /// Tabs are clickable when enabled.
  bool isClickable() const override { return isEnabled(); }

  /// Exposes each tab as a surface-colored interaction rectangle.
  ColorRole containerRole() const override { return ColorRole::kSurface; }

  /// Returns the tab background color.
  Color background() const override;

  /// Selection changes content color only; tabs do not show activated fill.
  bool useOverlayOnActivation() const override { return false; }

  /// Paints the tab-owned surface area, label, and optional icon foreground.
  void paint(PaintContext& ctx) const override;

  /// Reports the minimum label/icon cluster size plus token padding.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  Rect getCoreContentBounds() const;
  Rect getContentPaintBounds() const;
  Rect getDirectPaintExclusionBounds() const override;
  bool onSingleTapUp(XDim x, YDim y) override;
  void onClicked() override;
  virtual Dimensions getContentMinimumDimensions() const;
  virtual void paintContent(PaintContext& ctx, const Rect& content_bounds,
                            Color content_color) const;

 private:
  friend class Tabs;

  roo::string_view label_;
  const MonoIcon* icon_;
  uint8_t click_handled_on_release_ : 1;
};

class BadgedTab : public Tab {
 public:
  /// Creates a badged tab with the supplied non-owning label view.
  explicit BadgedTab(ApplicationContext& context, roo::string_view label = {});

  /// Returns the mutable inline badge helper.
  Badge& badge() { return badge_; }

  /// Returns the inline badge helper.
  const Badge& badge() const { return badge_; }

  /// Paints the tab content. Badge paint is deferred to a later phase.
  void paint(PaintContext& ctx) const override;

  /// Reports the base tab minimum size. Badge measurement is deferred.
  Dimensions getSuggestedMinimumDimensions() const override;

 private:
  Badge badge_;
};

class Tabs : public Container, private roo_scheduler::Executable {
 public:
  /// Creates a Material 3 tabs row with explicit variant and layout mode.
  explicit Tabs(ApplicationContext& context,
                TabsVariant variant = TabsVariant::kPrimary,
                TabsMode mode = TabsMode::kFixed);

  /// Detaches and deletes any owned tab children.
  ~Tabs() override;

  /// Returns the configured Material 3 tab visual variant.
  TabsVariant variant() const { return (TabsVariant)variant_; }

  /// Changes the configured visual variant.
  void setVariant(TabsVariant variant);

  /// Returns the configured layout mode.
  TabsMode mode() const { return (TabsMode)mode_; }

  /// Changes the layout mode. Scrollable mode currently falls back to fixed.
  void setMode(TabsMode mode);

  /// Returns whether the row should show its bottom divider.
  bool showsDivider() const { return shows_divider_; }

  /// Enables or disables the row-owned bottom divider.
  void setShowsDivider(bool shows_divider);

  /// Returns when touch selection is committed relative to click animation.
  TabsSelectionCommitMode selectionCommitMode() const {
    return (TabsSelectionCommitMode)selection_commit_mode_;
  }

  /// Configures when touch selection is committed relative to click animation.
  void setSelectionCommitMode(TabsSelectionCommitMode mode);

  /// Adds a borrowed `Tab` or subclass child to the row.
  template <typename T>
  void addTab(T& tab) {
    static_assert(std::is_base_of<Tab, T>::value,
                  "Tabs::addTab() only accepts Tab subclasses");
    addTabImpl(WidgetRef(tab), &tab);
  }

  /// Adds an owned `Tab` or subclass child to the row.
  template <typename T>
  void addTab(std::unique_ptr<T> tab) {
    static_assert(std::is_base_of<Tab, T>::value,
                  "Tabs::addTab() only accepts Tab subclasses");
    T* raw = tab.get();
    addTabImpl(WidgetRef(std::move(tab)), raw);
  }

  /// Removes all tabs and clears selection.
  void clearTabs();

  /// Returns the number of tabs in the row.
  int tabCount() const { return (int)tabs_.size(); }

  /// Returns the currently selected tab index, or -1 when empty.
  int selectedIndex() const { return selected_index_; }

  /// Selects a tab by index. Returns false for invalid or already-selected.
  bool setSelectedIndex(int index, bool animate = true);

  /// Exposes the tabs row as a surface-colored container.
  ColorRole effectiveContainerRole() const override {
    return ColorRole::kSurface;
  }

  /// Returns the row background color.
  Color background() const override;

  /// Paints the row-owned surface pixels not already settled by child tabs.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Called whenever an enabled tab is invoked, even if already selected.
  virtual void onTabInvoked(int index);

  /// Called after a successful selection change with state already updated.
  virtual void onSelectedIndexChanged(int old_index, int new_index);

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override { return tabCount(); }
  const Widget& getChild(int idx) const override { return *tabs_[idx]; }
  Widget& getChild(int idx) override { return *tabs_[idx]; }

 private:
  friend class Tab;

  enum class IndicatorAnimationState : uint8_t {
    kIdle,
    kAnimating,
  };

  void addTabImpl(WidgetRef tab, Tab* raw);
  void handleTabClicked(const Tab& tab);
  void updateActivatedStates();
  int findTabIndex(const Tab& tab) const;
  int rowHeight() const;
  int indicatorHeight() const;
  Rect dividerBounds() const;
  Rect indicatorBoundsForIndex(int index) const;
  Rect indicatorPaintBoundsForTab(const Tab& tab) const;
  Rect targetIndicatorBounds() const;
  void snapIndicatorToSelection();
  void startIndicatorTransition(const Rect& from, const Rect& to, bool animate);
  void cancelPendingIndicatorUpdate();
  void scheduleIndicatorUpdate();
  void execute(roo_scheduler::ExecutionID id) override;

  std::vector<Tab*> tabs_;
  roo_scheduler::Scheduler& scheduler_;
  roo_scheduler::ExecutionID notification_id_;
  Rect indicator_current_;
  Rect indicator_start_;
  Rect indicator_target_;
  unsigned long indicator_start_time_ms_;
  unsigned long indicator_end_time_ms_;
  int16_t selected_index_;
  uint8_t variant_ : 2;
  uint8_t mode_ : 2;
  uint8_t shows_divider_ : 1;
  uint8_t warned_scrollable_ : 1;
  uint8_t indicator_animation_state_ : 1;
  uint8_t selection_commit_mode_ : 1;
};

}  // namespace material3
}  // namespace roo_windows
