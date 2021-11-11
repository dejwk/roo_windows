#pragma once

#include <memory>
#include <vector>

#include "roo_material_icons.h"
#include "roo_windows/containers/navigation_rail.h"

namespace roo_windows {

// NavigationPanel has a navigation rail that activate sub-panes.
class NavigationPanel : public Panel {
 public:
  NavigationPanel(const Environment& env, Panel* parent, Box bounds)
      : Panel(env, parent, bounds), env_(env) {
    int16_t rail_width = 72;
    if (rail_width > bounds.width()) rail_width = bounds.width();
    rail_ = std::unique_ptr<NavigationRail>(new NavigationRail(
        env, this, Box(0, 0, rail_width - 1, bounds.height() - 1)));
    contents_ = std::unique_ptr<Panel>(
        new Panel(env, this, Box(rail_width, 0, bounds.xMax(), bounds.yMax())));
  }

  Panel* addPane(const roo_display::MaterialIcon& icon, std::string text) {
    int idx = panes_.size();
    rail_->addDestination(icon, text, [this, idx]() { setActive(idx); });
    Panel* p = new Panel(env_, contents_.get(), contents_->bounds());
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
    rail_->setActive(index);
    for (int i = 0; i < panes_.size(); ++i) {
      panes_[i]->setVisible(i == index);
    }
  }

 private:
  const Environment& env_;
  std::unique_ptr<NavigationRail> rail_;
  std::unique_ptr<Panel> contents_;
  std::vector<std::unique_ptr<Panel>> panes_;
};

}  // namespace roo_windows