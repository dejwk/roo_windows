#pragma once

#include <memory>

#include "roo_display/ui/alignment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon_with_caption.h"

namespace roo_windows {

class Destination;
class NavigationRail;

class Destination : public IconWithCaption {
 public:
  Destination(const Environment& env, const roo_display::Pictogram& icon,
              std::string text, int idx, std::function<void()> activator)
      : IconWithCaption(env, std::move(icon), std::move(text)),
        idx_(idx),
        activator_(std::move(activator)) {}

  bool useOverlayOnActivation() const override { return false; }
  bool useOverlayOnPressAnimation() const override { return true; }
  bool isClickable() const override { return true; }
  bool usesHighlighterColor() const override { return true; }

  void onClicked() override;

 private:
  const NavigationRail* rail() const;
  NavigationRail* rail();

  int idx_;
  std::function<void()> activator_;
};

class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(const Environment& env);

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(72),
                         PreferredSize::MatchParentHeight());
  }

  void addDestination(const roo_display::Pictogram& icon, std::string text,
                      std::function<void()> activator);

  // Returns the number of destinations.
  int size() const { return destinations_.size(); }

  // Returns the index of active destination.
  int getActive() const { return active_; }

  bool setActive(int index);

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class Destination;

  const Environment& env_;
  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  roo_display::VAlign alignment_;
  LabelVisibility label_visibility_;
  int active_;
  VerticalDivider divider_;

  std::vector<std::unique_ptr<Destination>> destinations_;
};

}  // namespace roo_windows
