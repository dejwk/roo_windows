#pragma once

#include <memory>
#include <vector>

#include "roo_material_icons.h"
#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/static_layout.h"

namespace roo_windows {

// NavigationPanel has a navigation rail that activate sub-panes.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(const Environment& env);

  ~NavigationPanel();

  void addPage(const roo_display::MaterialIcon& icon, std::string text, Widget* page);

  void setActive(int index);

 protected:
  void setParentBounds(const Box& parent_bounds) override;

 private:
  bool empty() const { return contents_.children().empty(); }
  size_t page_count() const { return contents_.children().size(); }
  Widget* page(int index) { return contents_.children()[index].get(); }
  const Widget& page(int index) const { return *contents_.children()[index]; }

  const Environment& env_;
  NavigationRail rail_;
  StaticLayout contents_;
};

}  // namespace roo_windows
