#pragma once

#include <vector>

#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/gravity.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

// All children are laid out independently of each other. Each child can be
// aligned to the parent's bounds in a specified way.
class AlignedLayout : public Panel {
 public:
  // Layout parameters plus last cached measure for a child.
  class ChildMeasure {
   public:
    ChildMeasure(roo_display::Alignment alignment)
        : alignment_(alignment), latest_() {}

    const roo_display::Alignment alignment() const { return alignment_; }

    const CachedMeasure& latest() const { return latest_; }
    CachedMeasure& latest() { return latest_; }

   private:
    roo_display::Alignment alignment_;
    CachedMeasure latest_;
  };

  AlignedLayout(ApplicationContext& context) : Panel(context) {}

  /// Appends a child with an explicit alignment relative to the parent's
  /// bounds. Default is middle-center.
  void add(WidgetRef child,
           roo_display::Alignment alignment = roo_display::kMiddle |
                                              roo_display::kCenter) {
    child_measures_.emplace_back(alignment);
    Panel::add(std::move(child));
  }

  /// Reports the bounding box that contains all measured children at their
  /// natural sizes (alignment plays no role yet at this stage).
  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::MatchParentHeight());
  }

 protected:
  /// Measures every child against the parent's constraints and caches the
  /// largest width/height as the layout's preferred size.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Places each child at its preferred size, positioned according to its
  /// individual alignment within `rect`.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows
