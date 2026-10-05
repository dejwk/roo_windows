#include <array>
#include <limits>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/internal/color_format.h"
#include "roo_windows/core/exclusion_filter.h"
#include "roo_windows/core/rounded_clip.h"

namespace roo_windows {
namespace internal {

// Exercises the production traversal with smaller budgets, without adding
// runtime configuration or instrumentation to each filter.
class ExclusionFilterTestPeer {
 public:
  template <typename Filler>
  static void Fill(ExclusionFilter& filter, Box bounds, uint8_t budget,
                   Filler* filler) {
    filter.fillRect(bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax(),
                    0, budget, filler);
  }
};

}  // namespace internal
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
void Draw(ExclusionFilter& filter, bool colored, int budget = -1,
          roo_display::DisplayOutput* target = nullptr) {
  int16_t x0[] = {0, kWidth / 2};
  int16_t y0[] = {0, 0};
  int16_t x1[] = {kWidth / 2 - 1, kWidth - 1};
  int16_t y1[] = {kHeight - 1, kHeight - 1};
  Color colors[] = {kPaint, kOtherPaint};
  if (budget >= 0) {
    ASSERT_NE(target, nullptr);
    if (colored) {
      roo_display::BufferedRectWriter writer(*target, BlendingMode::kSource);
      for (int i = 0; i < 2; ++i) {
        roo_display::BufferedRectWriterFillAdapter<
            roo_display::BufferedRectWriter>
            filler(writer, colors[i]);
        internal::ExclusionFilterTestPeer::Fill(
            filter, Box(x0[i], y0[i], x1[i], y1[i]), budget, &filler);
      }
    } else {
      roo_display::BufferedRectFiller filler(*target, kPaint,
                                             BlendingMode::kSource);
      internal::ExclusionFilterTestPeer::Fill(filter, kScreen, budget, &filler);
    }
  } else if (colored) {
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

// Verifies immediate fallback and shared budgets of one/eight preserve colors
// and single writes for large ordinary, masked, and mixed exclusion lists.
TEST(MaskedExclusionSubdivision, BoundedTraversalMatchesCoverageOracle) {
  const Box outer_bounds(-8, -6, 70, 52);
  const Box inner_bounds(3, -2, 60, 45);
  const BorderStyle outer_style(22, 16, 8, 14, SmallNumber(0.5));
  const BorderStyle inner_style(18, 12, 20, 4, SmallNumber(1.5));
  RoundedClip outer;
  RoundedClip inner;
  outer.reset(&outer, outer_bounds, outer_style);
  inner.reset(&inner, inner_bounds, inner_style);
  inner.parent = &outer;
  std::array<bool, kWidth * kHeight> opaque{};
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      opaque[y * kWidth + x] = internal::RoundedFillCoverage(
                                   outer_bounds, outer_style.corner_radii(),
                                   outer_style.outline_width(), x, y) == 255 &&
                               internal::RoundedFillCoverage(
                                   inner_bounds, inner_style.corner_radii(),
                                   inner_style.outline_width(), x, y) == 255;
    }
  }
  for (int kind = 0; kind < 3; ++kind) {
    for (int count : {0, 1, 8, 64, 256}) {
      std::vector<Box> rectangles;
      std::vector<MaskedExclusion> masks;
      for (int i = 0; i < count; ++i) {
        const int x = (i * 13) % 96 - 16;
        const int y = (i * 17) % 72 - 12;
        const Box box(x, y, x + i % 11, y + i % 19);
        if (kind == 0 || (kind == 2 && i % 2 == 0)) {
          rectangles.push_back(box);
        } else {
          masks.push_back({box, &inner});
        }
      }
      std::array<bool, kWidth * kHeight> excluded{};
      for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
          bool blocked = false;
          for (const Box& box : rectangles) blocked |= box.contains(x, y);
          for (const MaskedExclusion& mask : masks) {
            blocked |= mask.bounds.contains(x, y) && opaque[y * kWidth + x];
          }
          excluded[y * kWidth + x] = blocked;
        }
      }
      ExclusionUnion exclusions(nullptr, nullptr);
      exclusions.reset(
          rectangles.data(),
          rectangles.empty() ? rectangles.data()
                             : rectangles.data() + rectangles.size(),
          masks.data(),
          masks.empty() ? masks.data() : masks.data() + masks.size());
      for (int budget : {0, 1, 8}) {
        for (bool colored : {false, true}) {
          SCOPED_TRACE(testing::Message()
                       << "kind=" << kind << " count=" << count
                       << " budget=" << budget << " colored=" << colored);
          RectangleOutput output;
          ExclusionFilter filter(output, &exclusions);
          Draw(filter, colored, budget, &output);
          ExpectPixels(output, excluded, colored);
          if (count == 0) {
            EXPECT_EQ(output.rectangles.size(), colored ? 2u : 1u);
          }
        }
      }
    }
  }
}

// Verifies the ordinary-to-masked transition cannot start a fresh budget.
// One ordinary frame exhausts budget=1; the remaining rounded piece must have
// exactly the same band decomposition as immediate iterative fallback.
TEST(MaskedExclusionSubdivision, TransitionSharesTheRemainingBudget) {
  RoundedClip clip;
  const Box bounds(16, 8, 47, 39);
  clip.reset(&clip, bounds, BorderStyle(10, 0));
  const MaskedExclusion mask{bounds, &clip};
  ExclusionUnion exclusions(nullptr, nullptr);
  exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
  RectangleOutput immediate;
  ExclusionFilter direct(immediate, &exclusions);
  Draw(direct, false, 0, &immediate);
  RectangleOutput transition;
  ExclusionFilter filter(transition, &exclusions);
  Draw(filter, false, 1, &transition);
  ASSERT_EQ(immediate.rectangles.size(), transition.rectangles.size());
  for (size_t i = 0; i < immediate.rectangles.size(); ++i) {
    EXPECT_EQ(immediate.rectangles[i].bounds, transition.rectangles[i].bounds);
  }
}

// Verifies the default eight-frame limit reaches fallback after seven ordinary
// exclusions and the transition frame, rather than opening a new masked budget.
TEST(MaskedExclusionSubdivision, DefaultBudgetStopsAtEightFrames) {
  std::array<Box, 7> rectangles;
  for (int i = 0; i < 7; ++i) rectangles[i] = Box(2 * i, 0, 2 * i, 47);
  RoundedClip clip;
  const Box bounds(24, 8, 47, 39);
  clip.reset(&clip, bounds, BorderStyle(10, 0));
  const MaskedExclusion mask{bounds, &clip};
  ExclusionUnion exclusions(rectangles.data(),
                            rectangles.data() + rectangles.size());
  exclusions.reset(rectangles.data(), rectangles.data() + rectangles.size(),
                   &mask, &mask + 1);
  RectangleOutput expected;
  ExclusionFilter fallback(expected, &exclusions);
  {
    roo_display::BufferedRectFiller filler(expected, kPaint,
                                           BlendingMode::kSource);
    for (int i = 0; i < 6; ++i) filler.fillRect(2 * i + 1, 0, 2 * i + 1, 47);
    internal::ExclusionFilterTestPeer::Fill(fallback, Box(13, 0, 63, 47), 0,
                                            &filler);
  }
  RectangleOutput actual;
  ExclusionFilter filter(actual, &exclusions);
  Draw(filter, false);
  ASSERT_EQ(expected.rectangles.size(), actual.rectangles.size());
  for (size_t i = 0; i < expected.rectangles.size(); ++i) {
    EXPECT_EQ(expected.rectangles[i].bounds, actual.rectangles[i].bounds);
  }
}

// Verifies the iterative fallback keeps a rounded mask's straight middle in
// two tall rectangles even though its corners vary from row to row.
TEST(MaskedExclusionSubdivision, IterativeFallbackBatchesMaskedMiddle) {
  RoundedClip clip;
  clip.reset(&clip, kScreen, BorderStyle(16, 0));
  const MaskedExclusion mask{Box(24, 0, 39, 47), &clip};
  ExclusionUnion exclusions(nullptr, nullptr);
  exclusions.reset(nullptr, nullptr, &mask, &mask + 1);
  RectangleOutput output;
  ExclusionFilter filter(output, &exclusions);
  {
    roo_display::BufferedRectFiller filler(output, kPaint,
                                           BlendingMode::kSource);
    internal::ExclusionFilterTestPeer::Fill(filter, Box(0, 18, 63, 29), 0,
                                            &filler);
  }
  ASSERT_EQ(output.rectangles.size(), 2u);
  EXPECT_EQ(output.rectangles[0].bounds, Box(0, 18, 23, 29));
  EXPECT_EQ(output.rectangles[1].bounds, Box(40, 18, 63, 29));
}

// Verifies fallback batches full-height strips, keeps full visibility and
// coverage fast paths, and handles union coverage made of adjacent exclusions.
TEST(MaskedExclusionSubdivision, IterativeFallbackKeepsBulkOutput) {
  const std::array<Box, 2> adjacent = {Box(16, 0, 31, 47), Box(32, 0, 47, 47)};
  ExclusionUnion exclusions(adjacent.data(), adjacent.data() + adjacent.size());
  RectangleOutput output;
  ExclusionFilter filter(output, &exclusions);
  Draw(filter, false, 0, &output);
  ASSERT_EQ(output.rectangles.size(), 2u);
  EXPECT_EQ(output.rectangles[0].bounds, Box(0, 0, 15, 47));
  EXPECT_EQ(output.rectangles[1].bounds, Box(48, 0, 63, 47));
  const Box covered(16, 0, 47, 47);
  output.rectangles.clear();
  {
    roo_display::BufferedRectFiller filler(output, kPaint,
                                           BlendingMode::kSource);
    internal::ExclusionFilterTestPeer::Fill(filter, covered, 0, &filler);
  }
  EXPECT_TRUE(output.rectangles.empty());
  exclusions.reset(&covered, &covered + 1);
  {
    roo_display::BufferedRectFiller filler(output, kPaint,
                                           BlendingMode::kSource);
    internal::ExclusionFilterTestPeer::Fill(filter, covered, 0, &filler);
  }
  EXPECT_TRUE(output.rectangles.empty());
}

// Verifies row and span arithmetic crosses the complete signed coordinate
// range without wrapping, including runs ending at INT16_MAX on either axis.
TEST(MaskedExclusionSubdivision, IterativeFallbackClampsWideCoordinates) {
  constexpr int16_t low = std::numeric_limits<int16_t>::min();
  constexpr int16_t high = std::numeric_limits<int16_t>::max();
  for (bool horizontal : {false, true}) {
    const Box bounds =
        horizontal ? Box(low, high, high, high) : Box(high, low, high, high);
    const Box excluded =
        horizontal ? Box(-2, high, 2, high) : Box(high, -2, high, 2);
    ExclusionUnion exclusions(&excluded, &excluded + 1);
    RectangleOutput output;
    ExclusionFilter filter(output, &exclusions);
    {
      roo_display::BufferedRectFiller filler(output, kPaint,
                                             BlendingMode::kSource);
      internal::ExclusionFilterTestPeer::Fill(filter, bounds, 0, &filler);
    }
    ASSERT_EQ(output.rectangles.size(), 2u);
    EXPECT_EQ(output.rectangles[0].bounds,
              horizontal ? Box(low, high, -3, high) : Box(high, low, high, -3));
    EXPECT_EQ(output.rectangles[1].bounds,
              horizontal ? Box(3, high, high, high) : Box(high, 3, high, high));
  }
}

}  // namespace
}  // namespace roo_windows
