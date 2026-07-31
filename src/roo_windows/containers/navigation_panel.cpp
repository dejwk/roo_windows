#include "roo_windows/containers/navigation_panel.h"

#include <algorithm>
#include <memory>

#include "roo_logging.h"
#include "roo_windows/config.h"

namespace roo_windows {

NavigationPanel::NavigationPanel(ApplicationContext& context)
    : Panel(context),
      rail_(context, *this),
      contents_(context),
      rail_width_(0) {
  destination_labels_.reserve(material3::NavigationRail::kMaxDestinations);
  add(rail_);
  add(contents_);
}

void NavigationPanel::addPage(const MonoIcon& icon, std::string text,
                              WidgetRef page) {
  CHECK_LT(page_count(), material3::NavigationRail::kMaxDestinations)
      << "NavigationPanel supports at most "
      << static_cast<int>(material3::NavigationRail::kMaxDestinations)
      << " pages";
  const bool first = empty();
  destination_labels_.push_back(std::move(text));
  const std::string& label = destination_labels_.back();
  auto destination = std::make_unique<material3::NavigationRailDestination>(
      context(), roo::string_view(label.data(), label.size()), &icon);
  CHECK(rail_.add(WidgetRef(std::move(destination))));
  Widget& w = *page;
  contents_.add(std::move(page));
  if (first) {
    // Adding the first rail destination selects it without firing the
    // selection-change hook, because no page existed at that point yet.
    showPage(0);
  } else {
    w.setVisibility(roo_windows::Visibility::kGone);
  }
}

void NavigationPanel::setActive(int index) {
  if (index < 0 || static_cast<size_t>(index) >= page_count()) return;
  rail_.setSelectedIndex(index);
  // Selecting the already-selected index has no rail callback, but callers
  // may still use setActive() to restore the corresponding page visibility.
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

void NavigationPanel::PanelRail::onSelectedIndexChanged(int old_index,
                                                        int new_index) {
  (void)old_index;
  panel_.showPage(new_index);
}

Dimensions NavigationPanel::onMeasure(WidthSpec width, HeightSpec height) {
  // Let the rail resolve its token-backed collapsed width independently, then
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

}  // namespace roo_windows
