#pragma once

#include <string>
#include <vector>

#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"

namespace roo_windows {

class NavigationPanelTestAccess;

/// Legacy-compatible page host backed by the Material 3 navigation rail.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(ApplicationContext& context);

  /// Adds a new page accessible via the rail. The icon/caption become a new
  /// destination; the page widget is added to the stacked content area.
  void addPage(const MonoIcon& icon, std::string text, WidgetRef page);

  /// Switches to the page at `index`, both visually and in the rail's
  /// selection state.
  void setActive(int index);

 protected:
  /// Measures the rail and the active page side-by-side and reports the
  /// combined size.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Places the rail on the leading edge and the stacked content area in the
  /// remaining space.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class NavigationPanelTestAccess;

  class PanelRail : public material3::NavigationRail {
   public:
    PanelRail(ApplicationContext& context, NavigationPanel& panel)
        : material3::NavigationRail(context), panel_(panel) {}

   protected:
    void onSelectedIndexChanged(int old_index, int new_index) override;

   private:
    NavigationPanel& panel_;
  };

  bool empty() const { return contents_.children().empty(); }
  size_t page_count() const { return contents_.children().size(); }
  Widget* page(int index) { return contents_.children()[index]; }
  const Widget& page(int index) const { return *contents_.children()[index]; }
  void showPage(int index);

  PanelRail rail_;
  StackedLayout contents_;
  XDim rail_width_;
  // Material 3 destinations intentionally borrow their labels. Reserve the
  // rail's fixed maximum once so views stay valid as pages are appended.
  std::vector<std::string> destination_labels_;
};

}  // namespace roo_windows
