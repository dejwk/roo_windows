#pragma once

#include <memory>
#include <vector>

#include "roo_material_icons.h"
#include "roo_windows/containers/navigation_rail.h"

namespace roo_windows {

// NavigationPanel has a navigation rail that activate sub-panes.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(const Environment& env)
      : Panel(env), env_(env), rail_(env), contents_(env) {
    add(&rail_);
    add(&contents_);
  }

  ~NavigationPanel() {
    swap(0, nullptr);
    swap(1, nullptr);
  }

  Panel* addPane(const roo_display::MaterialIcon& icon, std::string text) {
    int idx = panes_.size();
    rail_.addDestination(icon, text, [this, idx]() { setActive(idx); });
    Panel* p = new Panel(env_);
    contents_.add(p, contents_.bounds());
    bool first = panes_.empty();
    panes_.emplace_back(p);
    if (first) {
      setActive(0);
    } else {
      p->setVisible(false);
    }
    return p;
  }

  void setActive(int index) {
    rail_.setActive(index);
    for (int i = 0; i < panes_.size(); ++i) {
      panes_[i]->setVisible(i == index);
    }
  }

 protected:
  void setParentBounds(const Box& parent_bounds) override {
    Panel::setParentBounds(parent_bounds);
    int16_t rail_width = 72;
    if (rail_width > parent_bounds.width()) rail_width = parent_bounds.width();
    rail_.moveTo(
        Box(0, 0, rail_width - 1, parent_bounds.height() - 1));
    contents_.moveTo(
        Box(rail_width, 0, parent_bounds.xMax(), parent_bounds.yMax()));
    for (auto& child : contents_.children()) {
      child->moveTo(contents_.bounds());
    }
  }

 private:
  const Environment& env_;
  NavigationRail rail_;
  Panel contents_;
  std::vector<std::unique_ptr<Panel>> panes_;
};

}  // namespace roo_windows
