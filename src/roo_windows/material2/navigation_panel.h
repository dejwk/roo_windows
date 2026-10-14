#pragma once

#include <string>

#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/material2/navigation_rail.h"
#include "roo_windows/material2/theme_mixin.h"

namespace roo_windows::material2 {

class NavigationPanelTestAccess;

/// Material 2 page host with a navigation rail and stacked page contents.
class NavigationPanel : public ThemeMixin<Panel> {
 public:
  NavigationPanel(ApplicationContext& context);

  /// Detaches the rail and page layout before their member storage is
  /// destroyed.
  ~NavigationPanel() override { removeAll(); }

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

  bool empty() const { return contents_.children().empty(); }
  size_t page_count() const { return contents_.children().size(); }
  Widget* page(int index) { return contents_.children()[index]; }
  const Widget& page(int index) const { return *contents_.children()[index]; }
  void showPage(int index);

  NavigationRail rail_;
  ThemeMixin<StackedLayout> contents_;
  XDim rail_width_;
};

}  // namespace roo_windows::material2
