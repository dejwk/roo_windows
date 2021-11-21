#include "navigation_rail.h"

namespace roo_windows {

bool Divider::paint(const Surface& s) {
  if (!isInvalidated()) return true;
  const Box box = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x20);
  s.drawObject(roo_display::FilledRect(box.xMax() - 1, box.yMin(),
                                       box.xMax() - 1, box.yMax(), color));
  color.set_a(0x10);
  s.drawObject(roo_display::FilledRect(box.xMax(), box.yMin(), box.xMax(),
                                       box.yMax(), color));
  return true;
}

Dimensions Divider::getSuggestedMinimumDimensions() const {
  return Dimensions(2, 0);
}

void Destination::onClicked() {
  if (rail()->setActive(idx_)) activator_();
  IconWithCaption::onClicked();
}

const NavigationRail* Destination::rail() const {
  return static_cast<const NavigationRail*>(parent());
}

NavigationRail* Destination::rail() {
  return static_cast<NavigationRail*>(parent());
}

NavigationRail::NavigationRail(const Environment& env)
    : Panel(env),
      env_(env),
      alignment_(roo_display::VAlign::Top()),
      active_(-1),
      divider_(env) {}

void NavigationRail::setParentBounds(const Box& parent_bounds) {
  Panel::setParentBounds(parent_bounds);
  divider_.moveTo(Box(parent_bounds.xMax() - 2, parent_bounds.yMin(),
                      parent_bounds.xMax() - 1, parent_bounds.yMax()));
}

void NavigationRail::addDestination(const roo_display::MaterialIcon& icon,
                                    std::string text,
                                    std::function<void()> activator) {
  int16_t width = bounds().width();
  Box box(4, 4, width - 8, width - 8);
  box = box.translate(0, size() * width);
  Destination* dest = new Destination(
      env_, icon, std::move(text), destinations_.size(), std::move(activator));
  add(dest, box);
  destinations_.emplace_back(dest);
}

bool NavigationRail::setActive(int index) {
  if (index == active_) return false;
  active_ = index;
  for (int i = 0; i < destinations_.size(); i++) {
    destinations_[i]->setActivated(i == index);
  }
  return true;
}

}  // namespace roo_windows
