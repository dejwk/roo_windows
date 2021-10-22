#pragma once

#include <memory>

#include "roo_display/ui/alignment.h"
#include "roo_material_icons.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class Destination;

class Divider : public Widget {
 public:
  using Widget::Widget;
  void paint(const Surface& s) override;
};

class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(Panel* parent, Box bounds);

  void addDestination(const roo_display::MaterialIcon& icon, std::string text,
                      std::function<void()> activator);

  // Returns the number of destinations.
  int size() const { return destinations_.size(); }

  // Returns the index of active destination.
  int getActive() const { return active_; }

  bool setActive(int index);

 private:
  friend class Destination;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  roo_display::VAlign alignment_;
  LabelVisibility label_visibility_;
  int active_;
  Divider divider_;

  std::vector<Destination*> destinations_;
};

}  // namespace roo_windows