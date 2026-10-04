#include <array>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/internal/color_format.h"
#include "roo_windows/core/exclusion_filter.h"
#include "roo_windows/core/rounded_clip.h"

namespace roo_windows {
namespace {

using internal::ExclusionFilter;
using internal::ExclusionUnion;
using internal::MaskedExclusion;
using internal::RoundedClip;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
constexpr int kWidth = 64;
constexpr int kHeight = 48;
constexpr Color kPaint(0xFF3879C6);
constexpr Color kOtherPaint(0xFF872901);
const Box kScreen(0, 0, kWidth - 1, kHeight - 1);

// Records the actual rectangles emitted by the filter's buffered writers.
class RectangleOutput : public roo_display::DisplayOutput {
 public:
  struct PaintedRect {
    Box bounds;
    Color color;
  };

  void setAddress(uint16_t, uint16_t, uint16_t, uint16_t,
                  BlendingMode) override {
    ADD_FAILURE() << "Rectangle output should stay batched";
  }

  void write(Color*, uint32_t) override {
    ADD_FAILURE() << "Rectangle output should stay batched";
  }

  void writePixels(BlendingMode, Color*, int16_t*, int16_t*,
                   uint16_t) override {
    ADD_FAILURE() << "Rectangle output should stay batched";
  }

  void fillPixels(BlendingMode, Color, int16_t*, int16_t*, uint16_t) override {
    ADD_FAILURE() << "Rectangle output should stay batched";
  }

  void writeRects(BlendingMode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      rectangles.push_back({Box(x0[i], y0[i], x1[i], y1[i]), colors[i]});
    }
  }

  void fillRects(BlendingMode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      rectangles.push_back({Box(x0[i], y0[i], x1[i], y1[i]), color});
    }
  }

  const ColorFormat& getColorFormat() const override { return format_; }

  std::vector<PaintedRect> rectangles;

 private:
  roo_display::Argb8888 color_mode_;
  roo_display::internal::ColorFormatImpl<roo_display::Argb8888,
                                         roo_io::kBigEndian>
      format_{color_mode_};
};

// Draws the screen through either rectangle entry point, preserving batch
// colors.
void Draw(ExclusionFilter& filter, bool colored) {
  int16_t x0[] = {0, kWidth / 2};
  int16_t y0[] = {0, 0};
  int16_t x1[] = {kWidth / 2 - 1, kWidth - 1};
  int16_t y1[] = {kHeight - 1, kHeight - 1};
  Color colors[] = {kPaint, kOtherPaint};
  if (colored) {
    filter.writeRects(BlendingMode::kSource, colors, x0, y0, x1, y1, 2);
  } else {
    x1[0] = kWidth - 1;
    filter.fillRects(BlendingMode::kSource, kPaint, x0, y0, x1, y1, 1);
  }
}

// Checks both the final colors and that each visible pixel is emitted once.
void ExpectPixels(const RectangleOutput& output,
                  const std::array<bool, kWidth * kHeight>& excluded,
                  bool colored) {
  std::array<int, kWidth * kHeight> writes{};
  std::array<Color, kWidth * kHeight> actual;
  actual.fill(Color(0));
  for (const RectangleOutput::PaintedRect& rect : output.rectangles) {
    ASSERT_FALSE(rect.bounds.empty());
    ASSERT_TRUE(kScreen.contains(rect.bounds));
    for (int y = rect.bounds.yMin(); y <= rect.bounds.yMax(); ++y) {
      for (int x = rect.bounds.xMin(); x <= rect.bounds.xMax(); ++x) {
        ++writes[y * kWidth + x];
        actual[y * kWidth + x] = rect.color;
      }
    }
  }
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const int i = y * kWidth + x;
      const Color expected =
          excluded[i] ? Color(0)
                      : (colored && x >= kWidth / 2 ? kOtherPaint : kPaint);
      EXPECT_EQ(writes[i], excluded[i] ? 0 : 1) << x << ',' << y;
      EXPECT_EQ(actual[i], expected) << x << ',' << y;
    }
  }
}

// Verifies an intruding rounded mask leaves large exterior rectangles intact,
// including areas separated by a regular exclusion processed before the mask.
TEST(MaskedExclusionSubdivision, ExteriorRegionsRemainWhole) {
  const Box regular(6, 0, 9, 47);
  const Box bounds(20, 12, 43, 35);
  RoundedClip clip;
  clip.reset(&clip, bounds, BorderStyle(10, 0));
  const MaskedExclusion mask{bounds, &clip};
  ExclusionUnion exclusions(&regular, &regular + 1);
  exclusions.reset(&regular, &regular + 1, &mask, &mask + 1);
  for (bool colored : {false, true}) {
    RectangleOutput output;
    ExclusionFilter filter(output, &exclusions);
    Draw(filter, colored);
    // The input split at x=32 does not cross either of these regions.
    for (const Box& whole : {Box(0, 0, 5, 47), Box(44, 12, 63, 35)}) {
      int matches = 0;
      for (const RectangleOutput::PaintedRect& rect : output.rectangles) {
        if (rect.bounds == whole) ++matches;
      }
      EXPECT_EQ(matches, 1);
    }
    std::array<bool, kWidth * kHeight> excluded{};
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        excluded[y * kWidth + x] =
            regular.contains(x, y) ||
            internal::RoundedFillCoverage(bounds, {10, 10, 10, 10}, 0, x, y) ==
                255;
      }
    }
    ExpectPixels(output, excluded, colored);
  }
}

// Verifies equal adjacent corner spans are emitted as one tall rectangle, not
// separate rows, while preserving every unmasked pixel and the batch colors.
TEST(MaskedExclusionSubdivision, EqualCornerSpansAreCombined) {
  const Box bounds(16, 0, 47, 47);
  RoundedClip clip;
  clip.reset(&clip, bounds, BorderStyle(14, 0));
  const MaskedExclusion mask{bounds, &clip};
  ExclusionUnion exclusions(nullptr, nullptr);
  exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
  for (bool colored : {false, true}) {
    RectangleOutput output;
    ExclusionFilter filter(output, &exclusions);
    Draw(filter, colored);
    bool has_tall_corner = false;
    for (size_t i = 0; i < output.rectangles.size(); ++i) {
      const Box& a = output.rectangles[i].bounds;
      if (bounds.contains(a) && a.yMax() < 14 && a.height() > 1) {
        has_tall_corner = true;
      }
      for (size_t j = 0; j < i; ++j) {
        const Box& b = output.rectangles[j].bounds;
        if (a.xMin() == b.xMin() && a.xMax() == b.xMax()) {
          EXPECT_NE(a.yMax() + 1, b.yMin());
          EXPECT_NE(b.yMax() + 1, a.yMin());
        }
      }
    }
    EXPECT_TRUE(has_tall_corner);
    std::array<bool, kWidth * kHeight> excluded{};
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        excluded[y * kWidth + x] =
            internal::RoundedFillCoverage(bounds, {14, 14, 14, 14}, 0, x, y) ==
            255;
      }
    }
    ExpectPixels(output, excluded, colored);
  }
}

// Verifies every piece, including gaps inside an earlier mask's bounding box,
// still checks later masks. Mask order does not change pixels or cause overlap.
TEST(MaskedExclusionSubdivision, OverlappingMasksAndRectangles) {
  const std::array<Box, 3> bounds = {Box(10, 4, 48, 39), Box(4, 10, 55, 44),
                                     Box(12, 2, 50, 42)};
  // Decoration trims radii to fit the bounds before computing coverage.
  const std::array<BorderStyle, 3> styles = {
      BorderStyle(10, 16, 6, 12, SmallNumber(0.5))
          .trim(bounds[0].width(), bounds[0].height()),
      BorderStyle(18, 0).trim(bounds[1].width(), bounds[1].height()),
      BorderStyle(16, 4, 10, 8, 0).trim(bounds[2].width(), bounds[2].height())};
  std::array<RoundedClip, 3> clips;
  for (size_t i = 0; i < clips.size(); ++i) {
    clips[i].reset(&clips[i], bounds[i], styles[i]);
  }
  clips[2].parent = &clips[0];
  const std::array<Box, 2> rectangles = {Box(0, 22, 63, 25),
                                         Box(18, 0, 20, 47)};
  std::array<MaskedExclusion, 3> masks = {
      MaskedExclusion{Box(0, 0, 40, 47), &clips[0]},
      MaskedExclusion{Box(20, 0, 63, 47), &clips[1]},
      MaskedExclusion{Box(0, 0, 63, 47), &clips[2]}};
  for (int order = 0; order < 6; ++order) {
    if (order != 0) std::swap(masks[order % 2], masks[order % 2 + 1]);
    for (bool colored : {false, true}) {
      SCOPED_TRACE(testing::Message()
                   << "order=" << order << " colored=" << colored);
      ExclusionUnion exclusions(rectangles.data(),
                                rectangles.data() + rectangles.size());
      exclusions.reset(rectangles.data(), rectangles.data() + rectangles.size(),
                       masks.data(), masks.data() + masks.size());
      RectangleOutput output;
      ExclusionFilter filter(output, &exclusions);
      Draw(filter, colored);
      std::array<bool, kWidth * kHeight> excluded{};
      for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
          bool blocked = false;
          for (const Box& rect : rectangles) blocked |= rect.contains(x, y);
          for (const MaskedExclusion& mask : masks) {
            if (!mask.bounds.contains(x, y)) continue;
            bool opaque = true;
            for (const RoundedClip* clip = mask.mask; clip != nullptr;
                 clip = clip->parent) {
              const size_t i = clip - clips.data();
              opaque &= internal::RoundedFillCoverage(
                            bounds[i], styles[i].corner_radii(),
                            styles[i].outline_width(), x, y) == 255;
            }
            blocked |= opaque;
          }
          excluded[y * kWidth + x] = blocked;
        }
      }
      ExpectPixels(output, excluded, colored);
    }
  }
}

}  // namespace
}  // namespace roo_windows
