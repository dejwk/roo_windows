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

  AlignedLayout(const Environment& env) : Panel(env) {}

  void add(WidgetRef child,
           roo_display::Alignment alignment = roo_display::kMiddle |
                                              roo_display::kCenter) {
    child_measures_.emplace_back(alignment);
    Panel::add(std::move(child));
  }

  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows
