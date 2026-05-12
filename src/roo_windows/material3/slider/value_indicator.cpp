#include "roo_windows/material3/slider/value_indicator.h"

#include <cmath>
#include <cstdio>

#include "roo_display/ui/text_label.h"
#include "roo_windows/config.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/number.h"
#include "roo_windows/material3/slider/slider_internal.h"

namespace roo_windows {
namespace material3 {

namespace {

using roo_display::Box;
using roo_display::StringViewLabel;

// Visual constants. Padding/gap values match the M3 spec for the Standard
// slider size; if more sizes are added later, route these through the
// SliderSize enum.
constexpr int16_t kPaddingH = Scaled(16);
constexpr int16_t kPaddingV = Scaled(12);
constexpr int16_t kGap = Scaled(4);  // gap between thumb and bubble
constexpr int16_t kCornerRadius = Scaled(20);
constexpr int16_t kBubbleMaxWidth = Scaled(96);
// Conservative caption-glyph height used when sizing the transient envelope.
constexpr int16_t kCaptionHeight = Scaled(16);

// Tolerance used by FormatDefault() to decide whether a value should be
// rendered as an integer; matches the slider's value-snapping tolerance.
constexpr float kIntegerTolerance = internal::kSliderStepDivisibilityTolerance;

// Inset, in pixels, between bounds() and the inscribed inner rectangle
// (which is solidly bubble-colored). Mirrors BorderStyle::getThickness()
// for outline_width == 0 and matching corner radius:
//     thickness = r - floor(r * 181 / 256)   (~= 0.293 * r)
// For kCornerRadius==Scaled(20) (==20 px on a 1x display) this evaluates to 5.
constexpr int16_t kCornerInset = kCornerRadius - (181 * kCornerRadius) / 256;

}  // namespace

// static
void ValueIndicatorBubble::MeasureBubbleSize(roo::string_view text,
                                             int16_t& bubble_width,
                                             int16_t& bubble_height) {
  StringViewLabel label(text, font_caption(), roo_display::color::Black);
  Box label_extents = label.anchorExtents();
  bubble_width = label_extents.width() + 2 * kPaddingH;
  bubble_height = label_extents.height() + 2 * kPaddingV;
}

// static
roo::string_view ValueIndicatorBubble::FormatDefault(float value, char* scratch,
                                                     size_t scratch_size) {
  if (scratch == nullptr || scratch_size == 0) return roo::string_view();
  int len = 0;
  if (!std::isfinite(value)) {
    len = snprintf(scratch, scratch_size, "?");
  } else if (std::fabs(value - roundf(value)) <= kIntegerTolerance) {
    // Render as an integer when the value is on (or essentially on) a step.
    len = snprintf(scratch, scratch_size, "%.0f", value);
  } else {
    // Render two decimals, then trim trailing zeroes and a dangling dot so
    // that "0.50" -> "0.5" and "1.00" never appears.
    len = snprintf(scratch, scratch_size, "%.2f", value);
    while (len > 0 && scratch[len - 1] == '0') --len;
    if (len > 0 && scratch[len - 1] == '.') --len;
    if ((size_t)len < scratch_size) scratch[len] = '\0';
  }
  if (len < 0) {
    scratch[0] = '\0';
    return roo::string_view();
  }
  size_t used = (size_t)len;
  if (used >= scratch_size) used = scratch_size - 1;
  return roo::string_view(scratch, used);
}

// static
Rect ValueIndicatorBubble::ConservativeBounds(
    int16_t parent_width, int16_t parent_height, int16_t thumb_overhang,
    SliderValueIndicatorBehavior behavior, SliderOrientation orientation) {
  if (behavior == SliderValueIndicatorBehavior::kHidden || parent_width <= 0 ||
      parent_height <= 0) {
    return Rect();
  }
  int16_t bw = kBubbleMaxWidth;
  int16_t bh = 2 * kPaddingV + kCaptionHeight;
  bool clamp = behavior == SliderValueIndicatorBehavior::kWithinBounds;
  if (orientation == SliderOrientation::kVertical) {
    if (clamp && bh > parent_height) bh = parent_height;
    int16_t top, bottom;
    if (clamp) {
      top = 0;
      bottom = parent_height - 1;
    } else {
      int16_t margin = bh / 2 + thumb_overhang;
      top = -margin;
      bottom = parent_height - 1 + margin;
    }
    int16_t right = -kGap - 1;
    int16_t left = right - bw + 1;
    return Rect(left, top, right, bottom);
  }
  if (clamp && bw > parent_width) bw = parent_width;
  int16_t left, right;
  if (clamp) {
    left = 0;
    right = parent_width - 1;
  } else {
    int16_t margin = bw / 2 + thumb_overhang;
    left = -margin;
    right = parent_width - 1 + margin;
  }
  int16_t top = -kGap - bh;
  int16_t bottom = -kGap - 1;
  return Rect(left, top, right, bottom);
}

// static
Rect ValueIndicatorBubble::EnvelopeForCenterRange(
    int16_t parent_width, int16_t parent_height, float center_min,
    float center_max, SliderValueIndicatorBehavior behavior,
    SliderOrientation orientation) {
  return EnvelopeForCenterRange(
      parent_width, parent_height, center_min, center_max, behavior,
      orientation, kBubbleMaxWidth, 2 * kPaddingV + kCaptionHeight);
}

// static
Rect ValueIndicatorBubble::EnvelopeForCenterRange(
    int16_t parent_width, int16_t parent_height, float center_min,
    float center_max, SliderValueIndicatorBehavior behavior,
    SliderOrientation orientation, int16_t bubble_width,
    int16_t bubble_height) {
  if (behavior == SliderValueIndicatorBehavior::kHidden || parent_width <= 0 ||
      parent_height <= 0 || bubble_width <= 0 || bubble_height <= 0) {
    return Rect();
  }
  if (center_min > center_max) std::swap(center_min, center_max);
  int16_t bw = bubble_width;
  int16_t bh = bubble_height;
  bool clamp = behavior == SliderValueIndicatorBehavior::kWithinBounds;
  if (orientation == SliderOrientation::kVertical) {
    if (clamp && bh > parent_height) bh = parent_height;
    int16_t t_min = (int16_t)floorf(center_min) - bh / 2 - 1;
    int16_t t_max = (int16_t)ceilf(center_max) - bh / 2 + 1;
    if (clamp) {
      int16_t t_clamp_min = 0;
      int16_t t_clamp_max = parent_height - bh;
      if (t_clamp_max < 0) t_clamp_max = 0;
      if (t_min < t_clamp_min) t_min = t_clamp_min;
      if (t_min > t_clamp_max) t_min = t_clamp_max;
      if (t_max < t_clamp_min) t_max = t_clamp_min;
      if (t_max > t_clamp_max) t_max = t_clamp_max;
    }
    int16_t right = -kGap - 1;
    int16_t left = right - bw + 1;
    int16_t top = t_min;
    int16_t bottom = t_max + bh - 1;
    return Rect(left, top, right, bottom);
  }
  if (clamp && bw > parent_width) bw = parent_width;
  // The bubble's left edge for a thumb at `center` is
  // `round(center) - bw / 2` (see layout()). Use floor/ceil with a 1-pixel
  // safety margin to be robust to rounding when the center range came from
  // float math.
  int16_t l_min = (int16_t)floorf(center_min) - bw / 2 - 1;
  int16_t l_max = (int16_t)ceilf(center_max) - bw / 2 + 1;
  if (clamp) {
    int16_t l_clamp_min = 0;
    int16_t l_clamp_max = parent_width - bw;
    if (l_clamp_max < 0) l_clamp_max = 0;
    if (l_min < l_clamp_min) l_min = l_clamp_min;
    if (l_min > l_clamp_max) l_min = l_clamp_max;
    if (l_max < l_clamp_min) l_max = l_clamp_min;
    if (l_max > l_clamp_max) l_max = l_clamp_max;
  }
  int16_t left = l_min;
  int16_t right = l_max + bw - 1;
  int16_t top = -kGap - bh;
  int16_t bottom = -kGap - 1;
  return Rect(left, top, right, bottom);
}

bool ValueIndicatorBubble::layout(int16_t parent_width, int16_t parent_height,
                                  float thumb_center,
                                  SliderOrientation orientation,
                                  roo::string_view text, bool clamp_to_bounds,
                                  roo_display::Color bubble_color,
                                  roo_display::Color text_color) {
  valid_ = false;
  bounds_ = Rect();
  if (parent_width <= 0 || parent_height <= 0) return false;

  // Measure the label so we can size the pill snugly around it. The label
  // is re-measured inside paint() to draw the text; this small duplication
  // keeps the geometry visible at layout time without storing measurement
  // state.
  int16_t bw;
  int16_t bh;
  MeasureBubbleSize(text, bw, bh);
  if (orientation == SliderOrientation::kVertical) {
    int16_t right = -kGap - 1;
    int16_t left = right - bw + 1;
    int16_t top = (int16_t)roundf(thumb_center) - bh / 2;
    if (clamp_to_bounds) {
      if (bh > parent_height) {
        top = 0;
        bh = parent_height;
      } else {
        if (top < 0) top = 0;
        if (top + bh > parent_height) top = parent_height - bh;
      }
    }
    if (bw <= 0 || bh <= 0) return false;

    bounds_ = Rect(left, top, right, top + bh - 1);
    text_ = text;
    bubble_color_ = bubble_color;
    text_color_ = text_color;
    valid_ = true;
    return true;
  }

  int16_t left = (int16_t)roundf(thumb_center) - bw / 2;
  int16_t top = -kGap - bh;
  if (clamp_to_bounds) {
    if (bw > parent_width) {
      // Bubble is wider than the slider; collapse to the slider's width and
      // let the text be horizontally centered (and clipped if necessary).
      left = 0;
      bw = parent_width;
    } else {
      if (left < 0) left = 0;
      if (left + bw > parent_width) left = parent_width - bw;
    }
  }
  if (bw <= 0 || bh <= 0) return false;

  bounds_ = Rect(left, top, left + bw - 1, top + bh - 1);
  text_ = text;
  bubble_color_ = bubble_color;
  text_color_ = text_color;
  valid_ = true;
  return true;
}

void ValueIndicatorBubble::paint(const Canvas& canvas) const {
  if (!valid_ || bounds_.empty()) return;

  // The inscribed inner rectangle: bounds() inset on every side by the
  // corner inset. This rectangle lies entirely inside the rounded shape's
  // straight (non-curved) area, so it is safe to fill it as a solid block.
  // The corner strips (between bounds() and this rectangle) are deliberately
  // left untouched here; the decoration overlay registered in decorate()
  // will alpha-composite the rounded mask on top of whatever the parent
  // paints there afterwards.
  Rect inner(bounds_.xMin() + kCornerInset, bounds_.yMin() + kCornerInset,
             bounds_.xMax() - kCornerInset, bounds_.yMax() - kCornerInset);
  if (inner.empty()) return;

  // Confine all drawing to the inner rectangle, intersected with the
  // inherited clip box. This is also a defensive safeguard against
  // out-of-range pixel writes when the bubble lands near the screen edge.
  Canvas sub = canvas;
  sub.clipToExtents(inner);
  if (sub.clip_box().empty()) return;

  // Set the canvas background to the bubble color so that the text label's
  // inter-glyph fill (which uses bgcolor) blends to the bubble color rather
  // than to the surrounding surface bg (which is typically white) - that
  // mismatch is what previously produced the "bubble overwritten white"
  // flicker.
  sub.set_bgcolor(bubble_color_);

  // Fast solid fill of the pill's interior. Far cheaper than a smooth
  // round-rect; the rounded edges come from the decoration overlay.
  sub.fillRect(inner, bubble_color_);

  // Centered label: position by anchor-center, then draw without a tile so
  // we do not refill any rectangle around the glyphs.
  StringViewLabel label(text_, font_caption(), text_color_);
  Box ext = label.anchorExtents();
  int16_t bubble_cx = (bounds_.xMin() + bounds_.xMax()) / 2;
  int16_t bubble_cy = (bounds_.yMin() + bounds_.yMax()) / 2;
  int16_t label_cx = (ext.xMin() + ext.xMax()) / 2;
  int16_t label_cy = (ext.yMin() + ext.yMax()) / 2;
  sub.drawObject(label, bubble_cx - label_cx, bubble_cy - label_cy);
}

void ValueIndicatorBubble::decorate(const Canvas& canvas, Clipper& clipper,
                                    const OverlaySpec& overlay_spec) const {
  if (!valid_ || bounds_.empty()) return;

  // Absolute (display) coordinates of the bubble's full bounds. The clipper
  // wrapper's addDecoration() takes a Rect (with absolute values here),
  // converts to a Box internally, and forwards to the underlying clipper.
  Rect absolute(canvas.dx() + bounds_.xMin(), canvas.dy() + bounds_.yMin(),
                canvas.dx() + bounds_.xMax(), canvas.dy() + bounds_.yMax());

  // 1) Register the rounded-rect decoration overlay covering the bubble's
  //    full extents. Elevation is 0 (no shadow). Outline width is 0; the
  //    outline color is the same as bg so Decoration treats the outline as
  //    absent and uses the corner radii as the bubble's silhouette.
  BorderStyle::CornerRadii radii{(uint8_t)kCornerRadius, (uint8_t)kCornerRadius,
                                 (uint8_t)kCornerRadius,
                                 (uint8_t)kCornerRadius};
  clipper.addDecoration(canvas.clip_box(), absolute, /*elevation=*/0,
                        overlay_spec, bubble_color_, radii,
                        /*outline_width=*/SmallNumber(0),
                        /*outline_color=*/bubble_color_);

  // 2) Add an exclusion for the inscribed inner rectangle. This protects the
  //    pill interior from overdraw by subsequent sibling/parent paints
  //    (which would otherwise erase the just-drawn bubble color and label
  //    text) while leaving the corner strips available for the parent to
  //    paint over. When the parent does paint over those strips, the
  //    decoration overlay above blends bubble color with parent bg
  //    according to the rounded-corner alpha mask, producing the rounded
  //    edges.
  Box inner_abs(absolute.xMin() + kCornerInset, absolute.yMin() + kCornerInset,
                absolute.xMax() - kCornerInset, absolute.yMax() - kCornerInset);
  Box clipped = Box::Intersect(inner_abs, canvas.clip_box());
  if (!clipped.empty()) clipper.addExclusion(clipped);
}

}  // namespace material3
}  // namespace roo_windows
