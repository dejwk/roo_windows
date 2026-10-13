#include "benchmarks/native_damage_filter.h"

#include <array>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/rounded_clip.h"

namespace roo_windows::benchmarks {
namespace {

using internal::DamageRegion;
using internal::ExclusionFilter;
using internal::ExclusionUnion;
using internal::MaskedExclusion;
using internal::RoundedClip;
using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
using roo_display::DisplayOutput;

constexpr int kWidth = 32;
constexpr int kHeight = 24;
const Box kFull(0, 0, kWidth - 1, kHeight - 1);

/// Records raw writes while retaining the offscreen pixel result.
class RecordingDevice final : public roo_display::OffscreenDevice<Argb8888> {
 public:
  /// Creates a small ARGB8888 recorder on @p pixels.
  explicit RecordingDevice(roo::byte* pixels)
      : OffscreenDevice(kWidth, kHeight, pixels, Argb8888()) {}

  /// Records the next streamed destination window.
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  /// Counts and forwards streamed colors.
  void write(Color* colors, uint32_t count) override {
    Record(count);
    OffscreenDevice::write(colors, count);
  }

  /// Counts and forwards a streamed fill.
  void fill(Color color, uint32_t count) override {
    Record(count);
    OffscreenDevice::fill(color, count);
  }

  /// Counts and forwards sparse colored pixels.
  void writePixels(BlendingMode mode, Color* colors, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(colors + i, 1);
    }
  }

  /// Counts and forwards sparse same-color pixels.
  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  /// Counts and forwards colored rectangles.
  void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    ++write_rect_batches;
    last_write_rect_count = count;
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(colors[i], (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  /// Counts and forwards same-color rectangles.
  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    ++fill_rect_batches;
    last_fill_rect_count = count;
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  std::array<uint16_t, kWidth * kHeight> writes{};
  int write_rect_batches = 0;
  int fill_rect_batches = 0;
  int last_write_rect_count = 0;
  int last_fill_rect_count = 0;

 private:
  void Record(uint32_t count) {
    while (count-- > 0) {
      ++writes[y_ * kWidth + x_];
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int left_ = 0;
  int right_ = 0;
  int x_ = 0;
  int y_ = 0;
};

/// Exercises every output family, including split streams and direct images.
void DrawMethod(DisplayOutput& out, int method) {
  const Color paint(0xFF123456);
  if (method == 0 || method == 1 || method == 7 || method == 8) {
    out.setAddress(0, 0, kWidth - 1, kHeight - 1, BlendingMode::kSource);
    std::array<Color, kWidth * kHeight> colors;
    colors.fill(paint);
    if (method == 0) {
      out.write(colors.data(), colors.size());
    } else if (method == 1) {
      out.fill(paint, colors.size());
    } else if (method == 7) {
      out.write(colors.data(), 19);
      out.write(colors.data() + 19, 237);
      out.write(colors.data() + 256, colors.size() - 256);
    } else {
      out.fill(paint, 19);
      out.fill(paint, 237);
      out.fill(paint, colors.size() - 256);
    }
  } else if (method == 2 || method == 3) {
    for (int16_t y = 0; y < kHeight; ++y) {
      std::array<int16_t, kWidth> xs;
      std::array<int16_t, kWidth> ys;
      std::array<Color, kWidth> colors;
      for (int16_t x = 0; x < kWidth; ++x) {
        xs[x] = x;
        ys[x] = y;
        colors[x] = paint;
      }
      if (method == 2) {
        out.writePixels(BlendingMode::kSource, colors.data(), xs.data(),
                        ys.data(), kWidth);
      } else {
        out.fillPixels(BlendingMode::kSource, paint, xs.data(), ys.data(),
                       kWidth);
      }
    }
  } else if (method == 4 || method == 5) {
    int16_t x0 = 0;
    int16_t y0 = 0;
    int16_t x1 = kWidth - 1;
    int16_t y1 = kHeight - 1;
    Color color = paint;
    if (method == 4) {
      out.writeRects(BlendingMode::kSource, &color, &x0, &y0, &x1, &y1, 1);
    } else {
      out.fillRects(BlendingMode::kSource, paint, &x0, &y0, &x1, &y1, 1);
    }
  } else {
    std::array<roo::byte, kWidth * kHeight * 4> source{};
    out.drawDirectRect(source.data(), kWidth * 4, 0, 0, kWidth - 1, kHeight - 1,
                       0, 0);
  }
}

/// Verifies a region against the compiled-gap oracle and raw write uniqueness.
void Compare(const DamageRegion& damage, const Box* semantic_begin,
             const Box* semantic_end, const MaskedExclusion* masked_begin,
             const MaskedExclusion* masked_end) {
  ExclusionUnion semantic(semantic_begin, semantic_end);
  semantic.reset(semantic_begin, semantic_end, masked_begin, masked_end);
  std::vector<Box> compiled_boxes;
  damage.appendComplement(kFull, compiled_boxes);
  if (semantic_begin != nullptr) {
    compiled_boxes.insert(compiled_boxes.end(), semantic_begin, semantic_end);
  }
  const Box* begin = compiled_boxes.empty() ? nullptr : compiled_boxes.data();
  const Box* end = compiled_boxes.empty()
                       ? nullptr
                       : compiled_boxes.data() + compiled_boxes.size();
  ExclusionUnion compiled(begin, end);
  compiled.reset(begin, end, masked_begin, masked_end);
  const bool has_semantic =
      semantic_begin != semantic_end || masked_begin != masked_end;
  for (int method = 0; method < 9; ++method) {
    std::array<roo::byte, kWidth * kHeight * 4> compiled_pixels{};
    std::array<roo::byte, kWidth * kHeight * 4> native_pixels{};
    RecordingDevice compiled_device(compiled_pixels.data());
    RecordingDevice native_device(native_pixels.data());
    ExclusionFilter compiled_filter(compiled_device, &compiled);
    NativeDamageFilter native_filter(native_device, damage, semantic,
                                     has_semantic);
    DrawMethod(compiled_filter, method);
    DrawMethod(native_filter, method);
    EXPECT_EQ(compiled_pixels, native_pixels) << "method " << method;
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        bool damaged = false;
        for (uint8_t i = 0; i < damage.size(); ++i) {
          damaged |= damage.at(i).contains(x, y);
        }
        const int expected = damaged && !semantic.contains(x, y) ? 1 : 0;
        const int index = y * kWidth + x;
        EXPECT_EQ(compiled_device.writes[index], expected)
            << "compiled method " << method << " pixel " << x << "," << y;
        EXPECT_EQ(native_device.writes[index], expected)
            << "native method " << method << " pixel " << x << "," << y;
      }
    }
  }
}

// Verifies every output family, overlapping damage and rounded semantic masks
// produce identical pixels without duplicate destination writes.
TEST(NativeDamageFilterTest, OverlapAndRoundedSemanticOracle) {
  DamageRegion damage;
  damage.add(Box(2, 2, 12, 8));
  damage.add(Box(9, 6, 22, 14));
  damage.add(Box(15, 3, 29, 10));
  damage.add(Box(4, 15, 27, 21));
  const Box semantic_box(11, 9, 18, 13);
  RoundedClip rounded;
  rounded.reset(&rounded, Box(6, 4, 25, 19), BorderStyle(5, 0));
  const MaskedExclusion masked{Box(6, 4, 25, 19), &rounded};
  Compare(damage, &semantic_box, &semantic_box + 1, &masked, &masked + 1);
}

// Verifies damage holes are still filtered when semantic exclusions are empty.
TEST(NativeDamageFilterTest, SparseDamageWithoutSemanticExclusions) {
  DamageRegion damage;
  damage.add(Box(1, 1, 3, 3));
  damage.add(Box(28, 20, 30, 22));
  Compare(damage, nullptr, nullptr, nullptr, nullptr);
}

// Verifies a single fully covered rectangle retains the no-gap control path.
TEST(NativeDamageFilterTest, SingleFullDamageControl) {
  DamageRegion damage;
  damage.add(kFull);
  Compare(damage, nullptr, nullptr, nullptr, nullptr);
}

// Keeps disjoint damage portions in one hardware rectangle batch.
TEST(NativeDamageFilterTest, BatchesDisjointRectangles) {
  DamageRegion damage;
  damage.add(Box(1, 1, 3, 3));
  damage.add(Box(28, 20, 30, 22));
  ExclusionUnion none(nullptr, nullptr);
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  NativeDamageFilter filter(device, damage, none, false);
  filter.fillRect(BlendingMode::kSource, kFull, Color(0xFF426281));
  EXPECT_EQ(device.fill_rect_batches, 1);
  EXPECT_EQ(device.last_fill_rect_count, 2);

  Color color(0xFF426281);
  int16_t x0 = kFull.xMin();
  int16_t y0 = kFull.yMin();
  int16_t x1 = kFull.xMax();
  int16_t y1 = kFull.yMax();
  filter.writeRects(BlendingMode::kSource, &color, &x0, &y0, &x1, &y1, 1);
  EXPECT_EQ(device.write_rect_batches, 1);
  EXPECT_EQ(device.last_write_rect_count, 2);
}

// Verifies crossing damage, capacity merges and empty unions across all output
// methods without relying on a particular complement decomposition.
TEST(NativeDamageFilterTest, CrossingAndEmptyDamageOracle) {
  DamageRegion crossing;
  crossing.add(Box(3, 10, 28, 12));
  crossing.add(Box(14, 1, 17, 22));
  crossing.add(Box(1, 2, 5, 7));
  crossing.add(Box(26, 16, 30, 21));
  crossing.add(Box(7, 19, 11, 23));
  Compare(crossing, nullptr, nullptr, nullptr, nullptr);
  DamageRegion empty;
  Compare(empty, nullptr, nullptr, nullptr, nullptr);
}

// Verifies randomized retained unions and semantic rectangles against the
// pixel/write oracle, including fragmented envelopes.
TEST(NativeDamageFilterTest, RandomizedRegionOracle) {
  uint32_t state = 0x5A17u;
  auto next = [&state]() {
    state = state * 1664525u + 1013904223u;
    return state;
  };
  for (int trial = 0; trial < 50; ++trial) {
    DamageRegion damage;
    for (int i = 0; i < 5; ++i) {
      const int x0 = next() % 32;
      const int y0 = next() % 24;
      const int x1 = std::min(31, x0 + static_cast<int>(next() % 10));
      const int y1 = std::min(23, y0 + static_cast<int>(next() % 8));
      damage.add(Box(x0, y0, x1, y1));
    }
    const Box semantic(10, 7, 20, 16);
    Compare(damage, &semantic, &semantic + 1, nullptr, nullptr);
  }
}

}  // namespace
}  // namespace roo_windows::benchmarks
