#include "navigation_rail.h"

#include "roo_windows/widgets/icon_with_caption.h"

namespace roo_windows {

class Destination : public IconWithCaption {
 public:
  Destination(NavigationRail* parent, Box bounds,
              const roo_display::MaterialIcon& icon, std::string text, int idx,
              std::function<void()> activator)
      : IconWithCaption(parent, bounds, std::move(icon), std::move(text)),
        idx_(idx),
        activator_(std::move(activator)) {}

  bool useOverlayOnActivation() const override { return false; }
  bool useOverlayOnPressAnimation() const override { return true; }
  bool isClickable() const override { return true; }
  bool usesHighlighterColor() const override { return true; }

  void onClicked() override {
    if (rail()->setActive(idx_)) activator_();
    IconWithCaption::onClicked();
  }

 private:
  const NavigationRail* rail() const {
    return static_cast<const NavigationRail*>(parent());
  }

  NavigationRail* rail() { return static_cast<NavigationRail*>(parent()); }

  int idx_;
  std::function<void()> activator_;
};

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

NavigationRail::NavigationRail(Panel* parent, Box bounds)
    : Panel(parent, bounds),
      alignment_(roo_display::VAlign::Top()),
      active_(-1),
      divider_(this, Box(bounds.xMax() - 2, bounds.yMin(), bounds.xMax() - 1,
                         bounds.yMax())) {}

void NavigationRail::addDestination(const roo_display::MaterialIcon& icon,
                                    std::string text,
                                    std::function<void()> activator) {
  int16_t width = bounds().width();
  Box box(4, 4, width - 8, width - 8);
  box = box.translate(0, size() * width);
  Destination* dest =
      new Destination(this, box, icon, std::move(text), destinations_.size(),
                      std::move(activator));
  destinations_.push_back(dest);
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
