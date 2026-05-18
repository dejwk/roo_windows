#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Legacy (pre-Material 3) horizontal slider widget.
///
/// Position is a 16-bit value covering the full range `[0, 65535]`. Supports
/// dragging, scroll gestures, and tap-to-set. For new code, prefer
/// `material3::Slider`, which exposes richer styling and discrete-stop
/// behavior.
class Slider : public BasicWidget {
 public:
  Slider(const Environment& env, uint16_t pos = 0)
      : BasicWidget(env), pos_(pos), is_dragging_(false) {}

  /// Paints the track and thumb based on the current position; press state
  /// is reflected via the standard widget overlay.
  void paint(const Canvas& canvas) const override;

  /// Reports the minimum size required for the track and thumb glyph.
  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  /// Snaps the position to the touch point and arms scroll-dragging.
  bool onDown(XDim x, YDim y) override;

  /// Treats a tap-up as a final position commit.
  bool onSingleTapUp(XDim x, YDim y) override;

  /// Captures the press location so the slider can show a static press
  /// overlay before scrolling starts.
  void onShowPress(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

  bool usesHighlighterColor() const override { return true; }

  /// Translates horizontal scroll deltas into position deltas while dragging.
  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  /// Resets the drag-armed state if the gesture was canceled mid-drag.
  void onCancel() override;

  PreferredSize getPreferredSize() const override;

  /// Returns the thumb center as the focal point for the press overlay.
  roo_display::FpPoint getPointOverlayFocus() const override;

  uint16_t getPos() const { return pos_; }

  /// Sets the position; uses the full 0..65535 range. Returns true if `pos`
  /// actually changed (in which case the slider is redrawn).
  bool setPos(uint16_t pos);

 private:
  uint16_t pos_;
  bool is_dragging_;
};

}  // namespace roo_windows