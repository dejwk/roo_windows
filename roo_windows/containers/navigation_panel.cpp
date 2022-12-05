#include "roo_windows/containers/navigation_panel.h"

namespace roo_windows {

NavigationPanel::NavigationPanel(const Environment& env)
    : Panel(env), env_(env), rail_(env), contents_(env) {
  add(rail_);
  add(contents_);
}

void NavigationPanel::addPage(const roo_display::MaterialIcon& icon,
                              std::string text, WidgetRef page) {
  int idx = page_count();
  rail_.addDestination(icon, text, [this, idx]() { setActive(idx); });
  bool first = empty();
  Widget& w = *page;
  contents_.add(std::move(page));
  if (first) {
    setActive(0);
  } else {
    w.setVisibility(GONE);
  }
}

void NavigationPanel::setActive(int index) {
  rail_.setActive(index);
  for (size_t i = 0; i < page_count(); ++i) {
    page(i)->setVisibility(i == (size_t)index ? VISIBLE : GONE);
  }
}

Dimensions NavigationPanel::onMeasure(WidthSpec width, HeightSpec height) {
  PreferredSize rail_size = rail_.getPreferredSize();
  Dimensions rail =
      rail_.measure(width.getChildWidthSpec(0, rail_size.width()),
                    height.getChildHeightSpec(0, rail_size.height()));
  WidthSpec w =
      width.getChildWidthSpec(rail.width(), PreferredSize::MatchParentWidth());
  HeightSpec h =
      height.getChildHeightSpec(0, PreferredSize::MatchParentHeight());
  Dimensions contents = contents_.measure(w, h);
  return Dimensions(rail.width() + contents.width(),
                    std::max(rail.height(), contents.height()));
}

void NavigationPanel::onLayout(bool changed, const Rect& rect) {
  rail_.layout(Rect(0, 0, 71, rect.yMax()));
  contents_.layout(Rect(72, 0, rect.xMax(), rect.yMax()));
}

}  // namespace roo_windows
