#pragma once

#include <memory>

#include "roo_display/filter/color_filter.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_material_icons.h"
#include "roo_windows/icon_with_caption.h"
#include "roo_windows/main_window.h"
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
        active_(-1) {}

  void addDestination(const roo_display::MaterialIconDef& icon,
                      std::string text) {
    int16_t width = bounds().width();
    Box box(4, 4, width - 8, width - 8);
    box = box.translate(0, children().size() * width);
    Destination* dest = new Destination(this, box, icon, std::move(text));
    int idx = destinations_.size();
    dest->setOnClicked([this, idx]{ setActive(idx); });
    destinations_.push_back(dest);
  }

  void paint(const roo_display::Surface& s) override {
    bool repaint = needs_repaint_;
    Panel::paint(s);
    if (repaint) {
      // Draw the divider.
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
    if (index == active_) return;
    active_ = index;
    for (int i = 0; i < destinations_.size(); i++) {
      destinations_[i]->setActivated(i == index);
    }
  }

 private:
  class Destination : public roo_windows::IconWithCaption {
   public:
    Destination(NavigationRail* parent, Box bounds,
                const roo_display::MaterialIconDef& icon, std::string text)
        : roo_windows::IconWithCaption(parent, bounds, std::move(icon), std::move(text)) {}

    bool useOverlayOnActivation() const override { return false; }
    bool useOverlayOnPressAnimation() const override { return true; }
    bool isClickable() const override { return true; }
    bool usesHighlighterColor() const override { return true; }

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