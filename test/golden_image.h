#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"

namespace roo_windows {
namespace test {

inline roo_display::Offscreen<roo_display::Rgb888> CaptureRgb(
    const roo_display::Rasterizable& source, int16_t x_min, int16_t y_min,
    int16_t width, int16_t height) {
  width = std::max<int16_t>(0, width);
  height = std::max<int16_t>(0, height);
  roo_display::Offscreen<roo_display::Rgb888> out(width, height,
                                                   roo_display::Rgb888());

  if (width == 0 || height == 0) return out;

  std::vector<int16_t> src_x(width);
  std::vector<int16_t> src_y(width);
  std::vector<int16_t> dst_x(width);
  std::vector<int16_t> dst_y(width);
  std::vector<roo_display::Color> colors(width);

  for (int i = 0; i < width; ++i) {
    src_x[i] = x_min + i;
    dst_x[i] = i;
  }

  for (int row = 0; row < height; ++row) {
    std::fill(src_y.begin(), src_y.end(), y_min + row);
    source.readColors(src_x.data(), src_y.data(), width, colors.data());
    std::fill(dst_y.begin(), dst_y.end(), row);
    out.output().writePixels(roo_display::BlendingMode::kSource,
                             colors.data(), dst_x.data(), dst_y.data(),
                             width);
  }

  return out;
}

::testing::AssertionResult CompareOrUpdateGolden(
    const roo_display::Rasterizable& actual,
    const std::string& golden_relative_path,
    const std::string& artifact_stem);

}  // namespace test
}  // namespace roo_windows
