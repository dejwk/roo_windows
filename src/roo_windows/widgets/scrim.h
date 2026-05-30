#pragma once

#include "roo_display.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// A widget that applies a semi-transparent overlay over all content beneath it.
// The scrim does not add its own area to the exclusion region, so that
// everything beneath it gets redrawn with the overlay applied.
class Scrim : public Widget {
 public:
  Scrim(const Environment& env)
      : Widget(env),
        color_(roo_display::Color(0x80000000)),
        fill_(color_) {}

  Scrim(const Environment& env, roo_display::Color color)
      : Widget(env), color_(color), fill_(color) {}

  roo_display::Color color() const { return color_; }

  /// Reports a 0x0 minimum; the scrim derives its size entirely from the
  /// preferred-size match-parent contract below.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 0);
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::MatchParentHeight());
  }

 protected:
  /// Installs the scrim's fill as an overlay over the clip box rather than
  /// painting opaque pixels, so widgets beneath remain visible through it.
  void finalizePaintWidget(const Canvas& canvas,
                           Clipper& clipper) const override {
    clipper.addOverlay(&fill_, canvas.clip_box());
  }

 private:
  roo_display::Color color_;
  roo_display::Fill fill_;
};

}  // namespace roo_windows
