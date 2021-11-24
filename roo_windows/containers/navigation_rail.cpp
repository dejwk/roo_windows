#include "navigation_rail.h"

namespace roo_windows {

bool Divider::paint(const Surface& s) {
  if (!isInvalidated()) return true;
  const Box box = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  s.drawObject(roo_display::FilledRect(box.xMax() - 1, box.yMin(),
                                       box.xMax() - 1, box.yMax(), color));
  color.set_a(0x20);
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
      divider_(env) {
  add(&divider_);
}

NavigationRail::~NavigationRail() {
  // Remove the divider so that it doesn't get double-destroyed.
  swap(0, nullptr);
}

Dimensions NavigationRail::onMeasure(MeasureSpec width, MeasureSpec height) {
  return Dimensions(width.resolveSize(72),
                    height.resolveSize(72 * destinations_.size()));
}

void NavigationRail::onLayout(boolean changed, const roo_display::Box& box) {
  divider_.layout(Box(box.xMax() - 2, box.yMin(), box.xMax() - 1, box.yMax()));
  if (destinations_.empty()) return;
  int16_t dwidth = box.width() - 4;
  int16_t dheight = std::min<int16_t>(box.height() / destinations_.size(), 72);
  int16_t y = 0;
  for (const auto& d : destinations_) {
    d->layout(Box(4, y + 4, dwidth - 5, y + dheight - 5));
    y += dheight;
  }
}

void NavigationRail::addDestination(const roo_display::MaterialIcon& icon,
                                    std::string text,
                                    std::function<void()> activator) {
  Destination* dest = new Destination(
      env_, icon, std::move(text), destinations_.size(), std::move(activator));
  add(dest);
  destinations_.emplace_back(dest);
  dest->requestLayout();
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
