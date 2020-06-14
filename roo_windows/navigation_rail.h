#pragma once

#include <memory>

#include "roo_display/filter/color_filter.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_material_icons.h"
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
    Box box(4, 4, width - 4, width - 4);
    box = box.translate(0, children_.size() * width);
    Destination* dest = new Destination(this, box, icon, std::move(text));
    destinations_.push_back(dest);
  }

  void paint(const roo_display::Surface& s, bool repaint) override {
    Panel::paint(s, repaint);
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
      destinations_[i]->setActive(i == index);
    }
  }

  virtual bool onTouch(const TouchEvent& event) {
    // Find if can delegate to a child.
    int active = -1;
    for (int i = 0; i < children_.size(); ++i) {
      if (children_[i]->parent_bounds().contains(event.x(), event.y())) {
        active = i;
        break;
      }
    }
    if (active >= 0) setActive(active);
    return true;
  }

 private:
  class Destination : public Widget {
   public:
    enum State { ACTIVE, INACTIVE, FOCUSED, PRESSED };

    Destination(NavigationRail* parent, Box bounds,
                const roo_display::MaterialIconDef& icon, std::string text)
        : Widget(parent, bounds),
          icon_(std::move(icon)),
          text_(std::move(text)),
          active_(false) {}

    void setActive(bool active) {
      if (active_ != active) {
        markDirty();
      }
      active_ = active;
    }

    void paint(const Surface& s, bool repaint) override {
      Surface news = s;
      const Theme* theme = rail()->theme_;
      Color fg = active_ ? theme->color.primary : theme->color.onSurface;
      roo_display::MaterialIcon icon(icon_, fg);
      const roo_display::Font* font = theme->font.caption;
      int16_t total_height =
          icon.extents().height() + font->metrics().maxHeight();
      int16_t icon_x = (width() - icon.extents().width()) / 2;
      int16_t icon_y = (height() - total_height) / 2;
      roo_display::TranslucencyFilter filter(s.out, 0x40, theme->color.surface);
      if (!active_) {
        news.out = &filter;
      }
      news.dx = s.dx + icon_x;
      news.dy = s.dy + icon_y;
      news.drawObject(icon);
      auto label = roo_display::TextLabel(*font, text_, fg,
                                          roo_display::FILL_MODE_RECTANGLE);
      int16_t label_x = (width() - label.metrics().width()) / 2;
      int16_t label_y =
          icon_y + icon.extents().height() + font->metrics().ascent();
      news.dx = s.dx + label_x;
      news.dy = s.dy + label_y;
      news.drawObject(label);
    }

   private:
    const NavigationRail* rail() const {
      return (const NavigationRail*)parent();
    }

    const roo_display::MaterialIconDef& icon_;
    std::string text_;
    bool active_;
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