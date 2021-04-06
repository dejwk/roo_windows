#pragma once

#include <memory>

#include "roo_display/filter/color_filter.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_material_icons.h"
#include "roo_windows/icon_with_caption.h"
#include "roo_windows/panel.h"
#include "roo_windows/theme.h"

namespace roo_windows {

class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(Panel* parent, Box bounds)
      : Panel(parent, bounds),
        alignment_(roo_display::VAlign::Top()),
        theme_(&DefaultTheme()),
        active_(0) {}

  void addDestination(const roo_display::MaterialIconDef& icon,
                      std::string text) {
    int16_t width = bounds().width();
    Box box(4, 4, width - 8, width - 8);
    box = box.translate(0, children_.size() * width);
    Destination* dest = new Destination(this, box, icon, std::move(text));
    destinations_.push_back(dest);
  }

  void paint(const roo_display::Surface& s) override {
    bool repaint = needs_repaint_;
    Panel::paint(s);
    if (repaint) {
      const Box box = bounds();
      Color color = theme_->color.onBackground;
      color.set_a(0x20);
      s.drawObject(roo_display::FilledRect(box.xMax() - 2, box.yMin(),
                                           box.xMax() - 2, box.yMax(), color));
      color.set_a(0x10);
      s.drawObject(roo_display::FilledRect(box.xMax() - 1, box.yMin(),
                                           box.xMax() - 1, box.yMax(), color));
    }
  }

  int getActive() const { return active_; }

  void setActive(int index) {
    active_ = index;
    for (int i = 0; i < destinations_.size(); i++) {
      destinations_[i]->setActivated(i == index);
    }
  }

  bool onTouch(const TouchEvent& event) override {
    // Find if can delegate to a child.
    int active = -1;
    for (int i = 0; i < children_.size(); ++i) {
      if (children_[i]->parent_bounds().contains(event.x(), event.y())) {
        active = i;
        break;
      }
    }
    Panel::onTouch(event);
    if (active >= 0) {
      setActive(active);
    }
    return true;
  }

 private:
  class Destination : public roo_windows::IconWithCaption {
   public:
    Destination(NavigationRail* parent, Box bounds,
                const roo_display::MaterialIconDef& icon, std::string text)
        : roo_windows::IconWithCaption(parent, bounds, std::move(icon), std::move(text)) {}

    bool useOverlayOnActivation() override { return false; }

   private:
    const NavigationRail* rail() const {
      return (const NavigationRail*)parent();
    }
  };

  friend class Destination;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  roo_display::VAlign alignment_;
  LabelVisibility label_visibility_;
  const Theme* theme_;
  int active_;

  std::vector<Destination*> destinations_;
};

}  // namespace roo_windows