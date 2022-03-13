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
    w.setVisible(false);
  }
}

void NavigationPanel::setActive(int index) {
  rail_.setActive(index);
  for (size_t i = 0; i < page_count(); ++i) {
    page(i)->setVisible(i == (size_t)index);
  }
}

Dimensions NavigationPanel::onMeasure(MeasureSpec width, MeasureSpec height) {
  PreferredSize rail_size = rail_.getPreferredSize();
  Dimensions rail =
      rail_.measure(width.getChildMeasureSpec(0, rail_size.width()),
                    height.getChildMeasureSpec(0, rail_size.height()));
  MeasureSpec w =
      width.getChildMeasureSpec(rail.width(), PreferredSize::MatchParent());
  MeasureSpec h = width.getChildMeasureSpec(0, PreferredSize::MatchParent());
  Dimensions contents = contents_.measure(w, h);
  return Dimensions(rail.width() + contents.width(),
                    std::max(rail.height(), contents.height()));
}

void NavigationPanel::onLayout(bool changed, const roo_display::Box& box) {
  rail_.layout(Box(0, 0, 71, box.yMax()));
  contents_.layout(Box(72, 0, box.xMax(), box.yMax()));
}

}  // namespace roo_windows
