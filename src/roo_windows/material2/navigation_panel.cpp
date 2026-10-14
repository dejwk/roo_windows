#include "roo_windows/material2/navigation_panel.h"

#include <algorithm>

#include "roo_windows/config.h"

namespace roo_windows::material2 {

NavigationPanel::NavigationPanel(ApplicationContext& context)
    : ThemeMixin<Panel>(context),
      rail_(context),
      contents_(context),
      rail_width_(0) {
  add(rail_);
  add(contents_);
}

void NavigationPanel::addPage(const MonoIcon& icon, std::string text,
                              WidgetRef page) {
  const bool first = empty();
  const int index = page_count();
  rail_.addDestination(icon, std::move(text),
                       [this, index]() { showPage(index); });
  Widget& w = *page;
  contents_.add(std::move(page));
  if (first) {
    setActive(0);
  } else {
    w.setVisibility(roo_windows::Visibility::kGone);
  }
}

void NavigationPanel::setActive(int index) {
  if (index < 0 || static_cast<size_t>(index) >= page_count()) return;
  rail_.setActive(index);
  // Programmatic rail selection does not invoke a destination callback.
  // Always restore the corresponding page visibility, even on reselection.
  showPage(index);
}

void NavigationPanel::showPage(int index) {
  if (index < 0 || static_cast<size_t>(index) >= page_count()) return;
  for (size_t i = 0; i < page_count(); ++i) {
    page(i)->setVisibility(i == (size_t)index
                               ? roo_windows::Visibility::kVisible
                               : roo_windows::Visibility::kGone);
  }
}

Dimensions NavigationPanel::onMeasure(WidthSpec width, HeightSpec height) {
  // Let the rail resolve its width independently, then
  // give the stacked page host all remaining horizontal space.
  Dimensions rail = rail_.measure(
      WidthSpec::Unspecified(0),
      height.getChildHeightSpec(0, PreferredSize::MatchParentHeight()));
  rail_width_ = rail.width();
  WidthSpec w =
      width.getChildWidthSpec(rail.width(), PreferredSize::MatchParentWidth());
  HeightSpec h =
      height.getChildHeightSpec(0, PreferredSize::MatchParentHeight());
  Dimensions contents = contents_.measure(w, h);
  return Dimensions(rail.width() + contents.width(),
                    std::max(rail.height(), contents.height()));
}

void NavigationPanel::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  if (rect.empty()) {
    static_cast<Widget&>(rail_).layout(Rect(0, 0, -1, -1));
    contents_.layout(Rect(0, 0, -1, -1));
    return;
  }
  const XDim rail_width = std::min<XDim>(rect.width(), rail_width_);
  static_cast<Widget&>(rail_).layout(Rect(0, 0, rail_width - 1, rect.yMax()));
  contents_.layout(Rect(rail_width, 0, rect.xMax(), rect.yMax()));
}

}  // namespace roo_windows::material2
