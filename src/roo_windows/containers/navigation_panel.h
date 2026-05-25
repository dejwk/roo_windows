#pragma once

#include <memory>
#include <vector>

#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/stacked_layout.h"

namespace roo_windows {

// NavigationPanel has a navigation rail that activate sub-panes.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(ApplicationContext& context);

  /// Adds a new page accessible via the rail. The icon/caption become a new
  /// destination; the page widget is added to the stacked content area.
  void addPage(const roo_display::Pictogram& icon, std::string text,
               WidgetRef page);

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
  bool empty() const { return contents_.children().empty(); }
  size_t page_count() const { return contents_.children().size(); }
  Widget* page(int index) { return contents_.children()[index]; }
  const Widget& page(int index) const { return *contents_.children()[index]; }

  NavigationRail rail_;
  StackedLayout contents_;
};

}  // namespace roo_windows
