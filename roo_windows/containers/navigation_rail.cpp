#include "roo_windows/containers/navigation_rail.h"

#include "roo_windows/config.h"

namespace roo_windows {

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
      alignment_(roo_display::kTop),
      active_(-1),
      divider_(env) {
  add(divider_);
}

Dimensions NavigationRail::onMeasure(WidthSpec width, HeightSpec height) {
  return Dimensions(width.resolveSize(72),
                    height.resolveSize(72 * destinations_.size()));
}

void NavigationRail::onLayout(bool changed, const Rect& rect) {
  divider_.layout(
      Rect(rect.xMax() - 2, rect.yMin(), rect.xMax() - 1, rect.yMax()));
  if (destinations_.empty()) return;
  int16_t dwidth = rect.width() - Scaled(4);
  int16_t dheight =
      std::min<YDim>(rect.height() / destinations_.size(), Scaled(72));
  int16_t y = 0;
  for (const auto& d : destinations_) {
    d->layout(Rect(Scaled(4), y + Scaled(4), dwidth - Scaled(4) - 1,
                   y + dheight - Scaled(4) - 1));
    y += dheight;
  }
}

void NavigationRail::addDestination(const roo_display::Pictogram& icon,
                                    std::string text,
                                    std::function<void()> activator) {
  Destination* dest = new Destination(
      env_, icon, std::move(text), destinations_.size(), std::move(activator));
  destinations_.emplace_back(dest);
  add(*dest);
}

bool NavigationRail::setActive(int index) {
  if (index == active_) return false;
  active_ = index;
  for (size_t i = 0; i < destinations_.size(); i++) {
    destinations_[i]->setActivated(i == (size_t)index);
  }
  return true;
}

}  // namespace roo_windows
