#include <array>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/exclusion_filter.h"

namespace roo_windows {
namespace {

using internal::ExclusionFilter;
using internal::ExclusionUnion;
using internal::MaskedExclusion;
using internal::RoundedClip;
using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
constexpr int kWidth = 64;
constexpr int kHeight = 48;
constexpr Color kPaint(0xFF3879C6);

// Counts actual device writes across every output entry point, after filters.
class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* data)
      : OffscreenDevice(kWidth, kHeight, data, Argb8888()) {}

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++address_windows;
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color value, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(value, count);
  }

  void writePixels(BlendingMode mode, Color* values, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color value, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(value, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* values, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(BlendingMode mode, Color value, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(value, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  std::array<uint16_t, kWidth * kHeight> writes{};
  int address_windows = 0;

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      EXPECT_GE(x_, 0);
      EXPECT_LT(x_, kWidth);
      EXPECT_GE(y_, 0);
      EXPECT_LT(y_, kHeight);
      if (x_ >= 0 && x_ < kWidth && y_ >= 0 && y_ < kHeight) {
        ++writes[y_ * kWidth + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }
  int16_t left_ = 0;
  int16_t right_ = 0;
  int16_t x_ = 0;
  int16_t y_ = 0;
};

// Computes exclusions from the decoration coverage oracle, without span tables.
bool Excluded(const std::vector<Box>& rectangles,
              const std::vector<MaskedExclusion>& masks, int16_t x, int16_t y,
              Box outer_bounds, BorderStyle outer_style, Box inner_bounds,
              BorderStyle inner_style) {
  for (const Box& box : rectangles) {
    if (box.contains(x, y)) return true;
  }
  for (const MaskedExclusion& mask : masks) {
    if (!mask.bounds.contains(x, y)) continue;
    if (internal::RoundedFillCoverage(outer_bounds, outer_style.corner_radii(),
                                      outer_style.outline_width(), x,
                                      y) == 255 &&
        internal::RoundedFillCoverage(inner_bounds, inner_style.corner_radii(),
                                      inner_style.outline_width(), x,
                                      y) == 255) {
      return true;
    }
  }
  return false;
}

Color PixelColor(int index) {
  return Color(255, index % 251, (index * 11) % 253, (index * 7) % 255);
}

// Exercises stream chunks crossing rows, sparse arrays, and rectangle batches.
void Draw(roo_display::DisplayOutput& out, int method) {
  if (method < 2) {
    out.setAddress(0, 0, kWidth - 1, kHeight - 1, BlendingMode::kSource);
    std::array<Color, 97> colors;
    for (int pos = 0; pos < kWidth * kHeight;) {
      int count = std::min<int>(colors.size(), kWidth * kHeight - pos);
      if (method == 0) {
        for (int i = 0; i < count; ++i) colors[i] = PixelColor(pos + i);
        out.write(colors.data(), count);
      } else {
        out.fill(kPaint, count);
      }
      pos += count;
    }
  } else if (method < 4) {
    for (int16_t y = 0; y < kHeight; ++y) {
      std::array<int16_t, kWidth> xs;
      std::array<int16_t, kWidth> ys;
      std::array<Color, kWidth> colors;
      for (int16_t x = 0; x < kWidth; ++x) {
        xs[x] = x;
        ys[x] = y;
        colors[x] = PixelColor(y * kWidth + x);
      }
      if (method == 2) {
        out.writePixels(BlendingMode::kSource, colors.data(), xs.data(),
                        ys.data(), kWidth);
      } else {
        out.fillPixels(BlendingMode::kSource, kPaint, xs.data(), ys.data(),
                       kWidth);
      }
    }
  } else {
    // Outside and intersecting mask bounds alternate in one buffered batch.
    int16_t x0[] = {0, 0, 32, 32};
    int16_t x1[] = {31, 31, 63, 63};
    int16_t y0[] = {0, 4, 0, 4};
    int16_t y1[] = {3, 47, 3, 47};
    Color colors[] = {kPaint, kPaint, kPaint, kPaint};
    if (method == 4) {
      out.writeRects(BlendingMode::kSource, colors, x0, y0, x1, y1, 4);
    } else {
      out.fillRects(BlendingMode::kSource, kPaint, x0, y0, x1, y1, 4);
    }
  }
}

// Verifies every output path against exact coverage, with rectangular and
// disconnected masked exclusions, nesting, asymmetry, and fractional outlines.
TEST(MaskedExclusion, AllOutputPathsMatchCoverage) {
  for (int scene = 0; scene < 12; ++scene) {
    const Box outer_bounds(scene % 4, 4, 63, 47);
    const BorderStyle outer_style(4 + scene, 18 - scene, 10, 16,
                                  SmallNumber(0.5));
    const Box inner_bounds(1, 5 + scene % 3, 61, 46);
    const BorderStyle inner_style(20 - scene, 8, 14, 12, 0);
    RoundedClip outer;
    RoundedClip inner;
    outer.reset(&outer, outer_bounds, outer_style);
    inner.reset(&inner, inner_bounds, inner_style);
    inner.parent = &outer;
    std::vector<Box> rectangles;
    if ((scene % 3) != 0) rectangles.push_back(Box(7, 0, 10, 40));
    std::vector<MaskedExclusion> masks;
    if ((scene % 4) != 0) {
      masks.push_back({Box(0, 4, 24, 47), &inner});
      masks.push_back({Box(39, 4, 63, 47), &inner});
    }
    ExclusionUnion exclusions(nullptr, nullptr);
    exclusions.reset(
        rectangles.data(),
        rectangles.empty() ? rectangles.data()
                           : rectangles.data() + rectangles.size(),
        masks.data(),
        masks.empty() ? masks.data() : masks.data() + masks.size());
    for (int method = 0; method < 6; ++method) {
      SCOPED_TRACE(testing::Message()
                   << "scene=" << scene << " method=" << method);
      std::array<roo::byte, kWidth * kHeight * 4> data{};
      RecordingDevice device(data.data());
      ExclusionFilter filter(device, &exclusions);
      Draw(filter, method);
      for (int16_t y = 0; y < kHeight; ++y) {
        for (int16_t x = 0; x < kWidth; ++x) {
          const bool excluded =
              Excluded(rectangles, masks, x, y, outer_bounds, outer_style,
                       inner_bounds, inner_style);
          Color actual;
          device.raster().readColors(&x, &y, 1, &actual);
          const Color expected = excluded ? Color(0)
                                          : (method == 0 || method == 2
                                                 ? PixelColor(y * kWidth + x)
                                                 : kPaint);
          EXPECT_EQ(actual, expected) << x << ',' << y;
          EXPECT_EQ(device.writes[y * kWidth + x], excluded ? 0 : 1)
              << x << ',' << y;
        }
      }
    }
  }
}

// Verifies streamed full-height interior and exterior bands retain large
// writes.
TEST(MaskedExclusion, StraightMiddleKeepsMultirowBatches) {
  RoundedClip clip;
  clip.reset(&clip, Box(0, 0, 99, 399), BorderStyle(16, 0));
  MaskedExclusion mask{Box(0, 0, 49, 399), &clip};
  ExclusionUnion exclusions(nullptr, nullptr);
  exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
  EXPECT_GT(exclusions.excludedPixelsFromRowStart(Box(0, 20, 49, 370), 20),
            50u * 300);
  EXPECT_EQ(exclusions.visiblePixelsFromRowStart(Box(50, 0, 99, 399), 0),
            50u * 400);
  EXPECT_TRUE(mask.contains(Box(20, 20, 40, 300)));
  EXPECT_FALSE(mask.contains(Box(0, 0, 3, 3)));
  EXPECT_FALSE(mask.contains(Box(50, 20, 60, 30)));
}

// Verifies uniform fills emit tall side strips through the mask's straight
// middle instead of opening a separate device address window on every row.
TEST(MaskedExclusion, UniformSideStripsRemainRectangles) {
  RoundedClip clip;
  clip.reset(&clip, Box(0, 0, 63, 47), BorderStyle(16, 0));
  MaskedExclusion mask{Box(24, 0, 39, 47), &clip};
  ExclusionUnion exclusions(nullptr, nullptr);
  exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
  for (bool colored : {false, true}) {
    std::array<roo::byte, kWidth * kHeight * 4> data{};
    RecordingDevice device(data.data());
    ExclusionFilter filter(device, &exclusions);
    int16_t x0 = 0;
    int16_t x1 = 63;
    int16_t y0 = 18;
    int16_t y1 = 29;
    Color value = kPaint;
    if (colored) {
      filter.writeRects(BlendingMode::kSource, &value, &x0, &y0, &x1, &y1, 1);
    } else {
      filter.fillRects(BlendingMode::kSource, value, &x0, &y0, &x1, &y1, 1);
    }
    EXPECT_EQ(device.address_windows, 2);
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        const bool visible = y >= 18 && y <= 29 && (x < 24 || x > 39);
        EXPECT_EQ(device.writes[y * kWidth + x], visible ? 1 : 0);
      }
    }
  }
}

// Verifies a fill crossing both corner regions keeps each visible middle strip
// in one rectangle, with the same rectangle count as the middle grows taller.
TEST(MaskedExclusion, CrossingCornersKeepsMiddleBatched) {
  class RectRecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
   public:
    RectRecordingDevice(int height, roo::byte* data)
        : OffscreenDevice(kWidth, height, data, Argb8888()) {}

    void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                    int16_t* x1, int16_t* y1, uint16_t count) override {
      for (uint16_t i = 0; i < count; ++i) {
        fillRects(mode, colors[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
      }
    }

    void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                   int16_t* x1, int16_t* y1, uint16_t count) override {
      for (uint16_t i = 0; i < count; ++i) {
        rectangles.emplace_back(x0[i], y0[i], x1[i], y1[i]);
      }
      OffscreenDevice::fillRects(mode, color, x0, y0, x1, y1, count);
    }

    std::vector<Box> rectangles;
  };

  const BorderStyle style(6, 10, 8, 12, 0);
  for (bool colored : {false, true}) {
    size_t short_count = 0;
    for (int height : {48, 400}) {
      SCOPED_TRACE(testing::Message()
                   << "height=" << height << " colored=" << colored);
      const Box bounds(16, 0, 47, height - 1);
      RoundedClip clip;
      clip.reset(&clip, bounds, style);
      MaskedExclusion mask{bounds, &clip};
      ExclusionUnion exclusions(nullptr, nullptr);
      exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
      std::vector<roo::byte> data(kWidth * height * 4);
      RectRecordingDevice device(height, data.data());
      ExclusionFilter filter(device, &exclusions);
      int16_t x0 = 0;
      int16_t x1 = kWidth - 1;
      int16_t y0 = 0;
      int16_t y1 = height - 1;
      Color value = kPaint;
      if (colored) {
        filter.writeRects(BlendingMode::kSource, &value, &x0, &y0, &x1, &y1, 1);
      } else {
        filter.fillRects(BlendingMode::kSource, value, &x0, &y0, &x1, &y1, 1);
      }
      if (height == 48) short_count = device.rectangles.size();
      EXPECT_EQ(device.rectangles.size(), short_count);
      for (const Box middle :
           {Box(0, 16, 15, height - 17), Box(48, 16, 63, height - 17)}) {
        int containing_rectangles = 0;
        for (const Box& rect : device.rectangles) {
          if (rect.contains(middle)) ++containing_rectangles;
        }
        EXPECT_EQ(containing_rectangles, 1);
      }
      // Every output rectangle is disjoint, including at corner/middle seams.
      for (size_t i = 0; i < device.rectangles.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
          EXPECT_FALSE(device.rectangles[i].intersects(device.rectangles[j]));
        }
      }
      for (int16_t y = 0; y < height; ++y) {
        for (int16_t x = 0; x < kWidth; ++x) {
          const bool excluded =
              internal::RoundedFillCoverage(bounds, style.corner_radii(), 0, x,
                                            y) == 255;
          Color actual;
          device.raster().readColors(&x, &y, 1, &actual);
          EXPECT_EQ(actual, excluded ? Color(0) : kPaint) << x << ',' << y;
        }
      }
    }
  }
}

// Verifies a corner-crossing exclusion is one descriptor per draw, irrespective
// of radius, and descriptors borrow one geometry table rather than copying it.
TEST(MaskedExclusion, StorageDoesNotExpandIntoCornerRows) {
  std::array<roo::byte, kWidth * kHeight * 4> data{};
  RecordingDevice device(data.data());
  for (int radius : {8, 16, 32, 64}) {
    internal::ClipperState state;
    internal::ClipperOutput output(state, device);
    const Box bounds(0, 0, 199, 199);
    RoundedClip& clip =
        output.prepareRoundedClip(&state, bounds, BorderStyle(radius, 0));
    output.activateRoundedClip(clip);
    for (int i = 0; i < 20; ++i) output.addExclusion(Box(0, 0, 100 + i, 199));
    ASSERT_EQ(output.maskedExclusions().size(), 20u);
    EXPECT_TRUE(output.exclusions().empty());
    for (const MaskedExclusion& exclusion : output.maskedExclusions()) {
      EXPECT_EQ(exclusion.mask, &clip);
    }
    EXPECT_EQ(sizeof(Box), 8u);
    output.deactivateRoundedClip();
  }
}

// Verifies only cheap positive coverage proofs prune rectangles and overlays;
// a corner overlay survives despite being inside the masked bounding box.
TEST(MaskedExclusion, PruningRetainsUnprovenCoverage) {
  std::array<roo::byte, kWidth * kHeight * 4> data{};
  RecordingDevice device(data.data());
  internal::ClipperState state;
  internal::ClipperOutput output(state, device);
  const Box bounds(0, 0, 63, 47);
  output.setBounds(bounds);
  output.addExclusion(Box(0, 0, 1, 1));
  output.addExclusion(Box(22, 22, 25, 25));
  const Color green(0xFF00FF00);
  auto corner = roo_display::SmoothFilledRoundRect(62, 0, 63, 1, 0, green);
  auto middle = roo_display::SmoothFilledRoundRect(22, 22, 25, 25, 0, green);
  output.addOverlay(&corner, bounds);
  output.addOverlay(&middle, bounds);
  RoundedClip& clip =
      output.prepareRoundedClip(&state, bounds, BorderStyle(16, 0));
  output.activateRoundedClip(clip);
  output.addExclusion(bounds);
  output.deactivateRoundedClip();
  ASSERT_EQ(output.exclusions().size(), 1u);
  EXPECT_EQ(output.exclusions()[0], Box(0, 0, 1, 1));
  ASSERT_EQ(output.maskedExclusions().size(), 1u);
  output.setAddress(0, 0, 63, 47, BlendingMode::kSource);
  output.fill(kPaint, 64 * 48);
  int16_t x = 63;
  int16_t y = 0;
  Color actual;
  device.raster().readColors(&x, &y, 1, &actual);
  EXPECT_EQ(actual, green);
  output.addExclusion(bounds);
  EXPECT_TRUE(output.maskedExclusions().empty());
  ASSERT_EQ(output.exclusions().size(), 1u);
  EXPECT_EQ(output.exclusions()[0], bounds);
}

// Verifies a new paint drops masks and deferred sources from the completed
// paint before reusing geometry; no stale exclusion or overlay affects output.
TEST(MaskedExclusion, FreshPaintClearsDescriptorsAndReusesGeometry) {
  std::array<roo::byte, kWidth * kHeight * 4> data{};
  RecordingDevice device(data.data());
  internal::ClipperState state;
  const Box bounds(0, 0, 63, 47);
  RoundedClip* geometry;
  {
    auto corner =
        roo_display::SmoothFilledRoundRect(0, 0, 3, 3, 0, Color(0xFF00FF00));
    internal::ClipperOutput output(state, device);
    geometry = &output.prepareRoundedClip(&state, bounds, BorderStyle(16, 0));
    output.activateRoundedClip(*geometry);
    output.addExclusion(bounds);
    output.addOverlay(&corner, bounds);
    output.deactivateRoundedClip();
    output.setBounds(bounds);
    output.setAddress(0, 0, 63, 47, BlendingMode::kSource);
    output.fill(kPaint, 64 * 48);
  }
  // The previous raster source is now gone. Even the old composition stack
  // must be discarded without sampling it.
  internal::ClipperOutput next(state, device);
  EXPECT_TRUE(next.exclusions().empty());
  EXPECT_TRUE(next.maskedExclusions().empty());
  EXPECT_EQ(nullptr, next.roundedClip(&state));
  EXPECT_EQ(geometry,
            &next.prepareRoundedClip(&state, bounds, BorderStyle(16, 0)));
  const Color replacement(0xFF334455);
  next.setBounds(bounds);
  next.setAddress(0, 0, 63, 47, BlendingMode::kSource);
  next.fill(replacement, 64 * 48);
  for (int16_t y = 0; y < 48; ++y) {
    for (int16_t x = 0; x < 64; ++x) {
      Color actual;
      device.raster().readColors(&x, &y, 1, &actual);
      EXPECT_EQ(actual, replacement) << x << ',' << y;
    }
  }
}

}  // namespace
}  // namespace roo_windows
