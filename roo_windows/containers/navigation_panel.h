#pragma once

#include <memory>
#include <vector>

#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/stacked_layout.h"

namespace roo_windows {

// NavigationPanel has a navigation rail that activate sub-panes.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(const Environment& env);

  void addPage(const roo_display::Pictogram& icon, std::string text,
               WidgetRef page);

  void setActive(int index);

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  bool empty() const { return contents_.children().empty(); }
  size_t page_count() const { return contents_.children().size(); }
  Widget* page(int index) { return contents_.children()[index]; }
  const Widget& page(int index) const { return *contents_.children()[index]; }

  const Environment& env_;
  NavigationRail rail_;
  StackedLayout contents_;
};

}  // namespace roo_windows
