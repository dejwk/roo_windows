#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "roo_display.h"

namespace roo_windows {
namespace test {

inline roo_display::Offscreen<roo_display::Rgb888> CaptureRgb(
    const roo_display::Rasterizable& source, int16_t x_min, int16_t y_min,
    int16_t width, int16_t height) {
  width = std::max<int16_t>(0, width);
  height = std::max<int16_t>(0, height);
  roo_display::Offscreen<roo_display::Rgb888> out(
      width, height, roo_display::color::Transparent, roo_display::Rgb888());

  if (width == 0 || height == 0) return out;

  roo_display::DrawingContext ctx(out, -x_min, -y_min);
  ctx.setBackgroundColor(roo_display::color::Transparent);
  ctx.setFillMode(roo_display::FillMode::kExtents);
  ctx.setBlendingMode(roo_display::BlendingMode::kSource);
  ctx.draw(source);

  return out;
}

::testing::AssertionResult CompareOrUpdateGolden(
    const roo_display::Rasterizable& actual,
    const std::string& golden_relative_path, const std::string& artifact_stem);

}  // namespace test
}  // namespace roo_windows
