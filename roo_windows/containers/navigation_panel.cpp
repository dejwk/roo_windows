#include "roo_windows/containers/navigation_panel.h"

namespace roo_windows {

NavigationPanel::NavigationPanel(const Environment& env)
    : Panel(env), env_(env), rail_(env), contents_(env) {
  add(&rail_);
  add(&contents_);
}

NavigationPanel::~NavigationPanel() {
  swap(0, nullptr);
  swap(1, nullptr);
}

void NavigationPanel::addPage(const roo_display::MaterialIcon& icon,
                              std::string text, Widget* page) {
  int idx = page_count();
  rail_.addDestination(icon, text, [this, idx]() { setActive(idx); });
  bool first = empty();
  contents_.add(page, contents_.bounds());
  if (first) {
    setActive(0);
  } else {
    page->setVisible(false);
  }
}

void NavigationPanel::setActive(int index) {
  rail_.setActive(index);
  for (int i = 0; i < page_count(); ++i) {
    page(i)->setVisible(i == index);
  }
}

void NavigationPanel::setParentBounds(const Box& parent_bounds) {
  Panel::setParentBounds(parent_bounds);
  int16_t rail_width = 72;
  if (rail_width > parent_bounds.width()) rail_width = parent_bounds.width();
  rail_.moveTo(Box(0, 0, rail_width - 1, parent_bounds.height() - 1));
  contents_.moveTo(
      Box(rail_width, 0, parent_bounds.xMax(), parent_bounds.yMax()));
  for (auto& child : contents_.children()) {
    child->moveTo(contents_.bounds());
  }
}

}  // namespace roo_windows
