#pragma once

#include <memory>

#include "roo_display/ui/alignment.h"
#include "roo_material_icons.h"
#include "roo_display/ui/text_label.h"

namespace roo_windows {

class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(Panel* parent, Box bounds) : Panel(parent, bounds),
  alignment_(roo_display::VAlign::Top()), theme_(&defaultTheme()) {}

  void addDestination(const MaterialIconDef& icon, std::string text) {
    int16_t width = bounds().width();
    Box box(4, 4, width - 4, width - 4);
    Serial.println(children_.size());
    box = box.translate(0, children_.size() * (width - 8));
    Serial.println(box.yMin());
    Destination* dest = new Destination(this, box, icon, std::move(text));
    destinations_.push_back(dest);
  }

  void setActive(int index) {
    for (int i = 0; i < destinations_.size(); i++) {
      destinations_[i]->setActive(i == index);
    }
  }

  virtual bool onTouch(const TouchEvent& event) {
    TouchEvent shifted(event.type(), event.x() - bounds().xMin(),
                       event.y() - bounds().yMin());
    // Find if can delegate to a child.
    int active = -1;
    for (int i = 0; i < children_.size(); ++i) {
      if (children_[i]->bounds().contains(shifted.x(), shifted.y())) {
        active = i;
        break;
      }
    }
    setActive(active);
    return true;
  }

 private:
  class Destination : public Widget {
   public:
    enum State { ACTIVE, INACTIVE, FOCUSED, PRESSED };

    Destination(NavigationRail* parent, Box bounds,
                const MaterialIconDef& icon, std::string text)
        : Widget(parent, bounds),
          icon_(std::move(icon)), text_(std::move(text)),
          active_(false) {}

    void setActive(bool active) {
      if (active_ != active) {
        markDirty();
      }
      active_ = active;
    }

    virtual void update(const roo_display::Surface& s) {
      const Theme* theme = rail()->theme_;
      Color fg = active_ ? theme->color.primary : theme->color.onSurface;
      MaterialIcon icon(icon_, fg);
      const Font* font = theme->font.caption;
      int16_t total_height =
          icon.extents().height() + font->metrics().maxHeight();
      Box box = bounds();
      int16_t icon_x = (box.width() - icon.extents().width()) / 2;
      int16_t icon_y = (box.height() - total_height) / 2;
      Surface news = s;
      if (news.clipToExtents(box) == Box::CLIP_RESULT_EMPTY) return;
      news.bgcolor = theme->color.surface;
      news.dx = s.dx + icon_x + box.xMin();
      news.dy = s.dy + icon_y + box.yMin();
      news.drawObject(icon);
      auto label = TextLabel(*font, text_, fg, FILL_MODE_RECTANGLE);
      int16_t label_x = (box.width() - label.metrics().width()) / 2;
      int16_t label_y =
          icon_y + icon.extents().height() + font->metrics().ascent();
      news.dx = s.dx + label_x + box.xMin();
      news.dy = s.dy + label_y + box.yMin();
      news.drawObject(label);
    }

   private:
    const NavigationRail* rail() const {
      return (const NavigationRail*)parent();
    }

    const MaterialIconDef& icon_;
    std::string text_;
    // State state_;
    bool active_;
  };

  friend class Destination;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  roo_display::VAlign alignment_;
  LabelVisibility label_visibility_;
  const Theme* theme_;

  std::vector<Destination*> destinations_;
};

}  // namespace roo_windows