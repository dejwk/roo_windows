#pragma once

#include <memory>

#include "roo_display/ui/alignment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon_with_caption.h"

namespace roo_windows {

class Destination;
class NavigationRail;

/// One entry inside a `NavigationRail`: an icon-with-caption that activates a
/// caller-supplied callback when clicked.
class Destination : public IconWithCaption {
 public:
  Destination(ApplicationContext& context, const roo_display::Pictogram& icon,
              std::string text, int idx, std::function<void()> activator)
      : IconWithCaption(context, std::move(icon), std::move(text)),
        idx_(idx),
        activator_(std::move(activator)) {}

  bool useOverlayOnActivation() const override { return false; }

  bool isClickable() const override { return true; }
  bool usesHighlighterColor() const override { return true; }

  /// Activates this destination in its rail and invokes the user-supplied
  /// callback to switch the application's content.
  void onClicked() override;

 private:
  const NavigationRail* rail() const;
  NavigationRail* rail();

  int idx_;
  std::function<void()> activator_;
};

/// Material navigation rail: a vertical strip of `Destination` icons used as
/// the primary navigation surface on tablet/large layouts.
///
/// Owns its destinations and tracks the currently active one. Configurable
/// label visibility (`PERSISTED`, `SELECTED`, `UNLABELED`) controls when the
/// caption text is shown.
class NavigationRail : public Panel {
 public:
  enum LabelVisibility { PERSISTED, SELECTED, UNLABELED };

  NavigationRail(ApplicationContext& context);

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(72),
                         PreferredSize::MatchParentHeight());
  }

  /// Appends a new destination at the end of the rail.
  void addDestination(const roo_display::Pictogram& icon, std::string text,
                      std::function<void()> activator);

  /// Returns the number of destinations.
  int size() const { return destinations_.size(); }

  /// Returns the index of the currently active destination, or -1 if none.
  int getActive() const { return active_; }

  /// Sets the active destination by index. Returns true if the active
  /// destination changed.
  bool setActive(int index);

 protected:
  /// Measures every destination in a vertical column and reports the total
  /// rail size.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Stacks destinations vertically; positions them top, center, or bottom
  /// according to the configured alignment.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class Destination;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  roo_display::VAlign alignment_;
  LabelVisibility label_visibility_;
  int active_;
  VerticalDivider divider_;

  std::vector<std::unique_ptr<Destination>> destinations_;
};

}  // namespace roo_windows
