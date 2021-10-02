#pragma once

#include <memory>

#include "roo_display/ui/alignment.h"
#include "roo_material_icons.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class Destination;

class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(Panel* parent, Box bounds);

  void addDestination(const roo_display::MaterialIcon& icon, std::string text,
                      std::function<void()> activator);

  void paintWidget(const roo_display::Surface& s) override;

  int getActive() const { return active_; }

  bool setActive(int index);

 private:
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