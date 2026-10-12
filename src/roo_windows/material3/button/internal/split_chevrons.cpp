#include "roo_windows/material3/button/internal/split_chevrons.h"

#include <algorithm>
#include <cmath>

namespace roo_windows::material3::internal {

SplitChevron::SplitChevron() : image_(roo_display::Box(0, 0, -1, -1)) {
  image_.reserveInputs(2);
  image_.addInput(&outer_);
  image_.addInput(&cutout_);
}

void SplitChevron::setGeometry(roo_display::Box bounds, int slot,
                               float fraction, roo_display::Color color) {
  constexpr float kPi = 3.14159265358979323846f;
  constexpr float kSqrt2 = 1.4142135623730951f;
  float side = slot * 0.5f / kSqrt2;
  float stroke = slot / 12.0f;
  float turn = kPi * std::clamp(fraction, 0.0f, 1.0f);
  float sine = std::sin(turn);
  float cosine = std::cos(turn);
  float cx = (bounds.xMin() + bounds.xMax()) * 0.5f;
  float cy = (bounds.yMin() + bounds.yMax()) * 0.5f;
  // Subtracting an up-left shifted square leaves two perpendicular arms.
  // At 45 degrees their ink envelope is centered by this vertical offset.
  // Rotate that offset with the artwork so the pivot never follows its bounds.
  float offset = -(side - stroke) / (2.0f * kSqrt2);
  float cutout_offset = offset - kSqrt2 * stroke;
  outer_ = roo_display::SmoothRotatedFilledRect(
      {cx - sine * offset, cy + cosine * offset}, side, side, kPi / 4.0f - turn,
      color);
  cutout_ = roo_display::SmoothRotatedFilledRect(
      {cx - sine * cutout_offset, cy + cosine * cutout_offset}, side, side,
      kPi / 4.0f - turn, roo_display::color::Black);
  image_.setExtents(bounds);
  image_.setAnchorExtents(bounds);
  image_.setInput(0, &outer_);
  image_.setInput(1, &cutout_)
      .withMode(roo_display::BlendingMode::kDestinationOut);
}

}  // namespace roo_windows::material3::internal
