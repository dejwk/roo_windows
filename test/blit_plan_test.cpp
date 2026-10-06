#include "roo_windows/core/blit_plan.h"

#include <array>
#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/rasterizable.h"
#include "roo_display/internal/color_format.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {
namespace {

using internal::BlitPlan;
using internal::RoundedClip;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;

class NullOutput : public roo_display::DisplayOutput {
 public:
  void setAddress(uint16_t, uint16_t, uint16_t, uint16_t,
                  BlendingMode) override {}

  void write(Color*, uint32_t) override {}

  void writePixels(BlendingMode, Color*, int16_t*, int16_t*,
                   uint16_t) override {}

  void fillPixels(BlendingMode, Color, int16_t*, int16_t*, uint16_t) override {}

  void writeRects(BlendingMode, Color*, int16_t*, int16_t*, int16_t*, int16_t*,
                  uint16_t) override {}

  void fillRects(BlendingMode, Color, int16_t*, int16_t*, int16_t*, int16_t*,
                 uint16_t) override {}

  const ColorFormat& getColorFormat() const override { return format_; }

 private:
  roo_display::Argb8888 color_mode_;
  roo_display::internal::ColorFormatImpl<roo_display::Argb8888,
                                         roo_io::kBigEndian>
      format_{color_mode_};
};

class PlannerScene {
 public:
  struct MaskSpec {
    Box bounds;
    BorderStyle style;
  };

  struct MaskedRestrictionSpec {
    Box bounds;
    size_t mask_count;
  };

  PlannerScene() : clipper_(state_, output_) {}

  RoundedClip& pushMask(const void* owner, Box bounds, BorderStyle style) {
    RoundedClip& mask = clipper_.prepareRoundedClip(owner, bounds, style);
    clipper_.activateRoundedClip(mask);
    masks_.push_back(
        MaskSpec{bounds, style.trim(bounds.width(), bounds.height())});
    return mask;
  }

  void addBoxRestriction(Box restriction) {
    clipper_.addExclusion(restriction);
    if (masks_.empty()) {
      box_restrictions_.push_back(restriction);
    } else {
      masked_restrictions_.push_back(
          MaskedRestrictionSpec{restriction, masks_.size()});
    }
  }

  void addOverlayRestriction(const roo_display::Rasterizable& source,
                             Box clip) {
    clipper_.addOverlay(&source, clip);
    overlay_bounds_.push_back(Box::Intersect(source.extents(), clip));
  }

  BlitPlan plan(Box source_certificate, Box viewport, int16_t dx,
                int16_t dy) const {
    return clipper_.planBlitCopy(source_certificate, viewport, dx, dy);
  }

  bool opaqueThrough(size_t mask_count, int16_t x, int16_t y) const {
    for (size_t i = 0; i < mask_count; ++i) {
      const MaskSpec& mask = masks_[i];
      if (internal::RoundedFillCoverage(mask.bounds, mask.style.corner_radii(),
                                        mask.style.outline_width(), x,
                                        y) != 255) {
        return false;
      }
    }
    return true;
  }

  bool opaqueThrough(int16_t x, int16_t y) const {
    return opaqueThrough(masks_.size(), x, y);
  }

  const std::vector<Box>& boxRestrictions() const { return box_restrictions_; }

  const std::vector<MaskedRestrictionSpec>& maskedRestrictions() const {
    return masked_restrictions_;
  }

  const std::vector<Box>& overlayBounds() const { return overlay_bounds_; }

 private:
  NullOutput output_;
  internal::ClipperState state_;
  Clipper clipper_;
  std::vector<MaskSpec> masks_;
  std::vector<Box> box_restrictions_;
  std::vector<MaskedRestrictionSpec> masked_restrictions_;
  std::vector<Box> overlay_bounds_;
};

bool IsRestricted(const PlannerScene& scene, int16_t x, int16_t y) {
  for (const Box& exclusion : scene.boxRestrictions()) {
    if (exclusion.contains(x, y)) return true;
  }
  for (const Box& overlay : scene.overlayBounds()) {
    if (overlay.contains(x, y)) return true;
  }
  for (const PlannerScene::MaskedRestrictionSpec& exclusion :
       scene.maskedRestrictions()) {
    if (exclusion.bounds.contains(x, y) &&
        scene.opaqueThrough(exclusion.mask_count, x, y)) {
      return true;
    }
  }
  return false;
}

// Test-only pixel oracle. Production planning uses rectangles and row spans;
// this deliberately checks every selected destination independently.
bool IsCopyable(const PlannerScene& scene, Box source_certificate, Box viewport,
                int16_t dx, int16_t dy, int16_t x, int16_t y) {
  const int32_t source_x = static_cast<int32_t>(x) - dx;
  const int32_t source_y = static_cast<int32_t>(y) - dy;
  if (source_x < INT16_MIN || source_x > INT16_MAX || source_y < INT16_MIN ||
      source_y > INT16_MAX) {
    return false;
  }
  const int16_t sx = static_cast<int16_t>(source_x);
  const int16_t sy = static_cast<int16_t>(source_y);
  return viewport.contains(x, y) && viewport.contains(sx, sy) &&
         source_certificate.contains(sx, sy) && scene.opaqueThrough(x, y) &&
         scene.opaqueThrough(sx, sy) && !IsRestricted(scene, x, y) &&
         !IsRestricted(scene, sx, sy);
}

void ExpectPlanSafe(const PlannerScene& scene, const BlitPlan& plan,
                    Box source_certificate, Box viewport, int16_t dx,
                    int16_t dy) {
  ASSERT_FALSE(plan.empty());
  EXPECT_EQ(plan.source.width(), plan.destination.width());
  EXPECT_EQ(plan.source.height(), plan.destination.height());
  EXPECT_EQ(plan.source.xMin() + dx, plan.destination.xMin());
  EXPECT_EQ(plan.source.yMin() + dy, plan.destination.yMin());
  for (int32_t y = plan.destination.yMin(); y <= plan.destination.yMax(); ++y) {
    for (int32_t x = plan.destination.xMin(); x <= plan.destination.xMax();
         ++x) {
      ASSERT_TRUE(IsCopyable(scene, source_certificate, viewport, dx, dy,
                             static_cast<int16_t>(x), static_cast<int16_t>(y)))
          << "unsafe destination " << x << ", " << y;
    }
  }
}

void ExpectPlanAvoidsMaskedBounds(const PlannerScene& scene,
                                  const BlitPlan& plan, int16_t dx,
                                  int16_t dy) {
  for (const PlannerScene::MaskedRestrictionSpec& exclusion :
       scene.maskedRestrictions()) {
    EXPECT_FALSE(plan.destination.intersects(exclusion.bounds));
    const Box translated(
        exclusion.bounds.xMin() + dx, exclusion.bounds.yMin() + dy,
        exclusion.bounds.xMax() + dx, exclusion.bounds.yMax() + dy);
    EXPECT_FALSE(plan.destination.intersects(translated));
  }
}

// Verifies the core builders preserve the full translated overlap when there
// is no rounded geometry to trim. This covers the former raw-overlap candidate.
TEST(BlitPlanTest, PreservesFullOverlapWithoutRoundedMasks) {
  PlannerScene scene;
  const Box viewport(0, 0, 95, 63);

  const BlitPlan plan = scene.plan(viewport, viewport, 4, -3);

  EXPECT_EQ(Box(0, 3, 91, 63), plan.source);
  EXPECT_EQ(Box(4, 0, 95, 60), plan.destination);
  ExpectPlanSafe(scene, plan, viewport, viewport, 4, -3);
}

// Verifies the reference radius-16 scene retains the complete balanced core at
// both endpoints of an eight-pixel upward scroll.
TEST(BlitPlanTest, RecoversReferenceRoundedInterior) {
  PlannerScene scene;
  const Box viewport(0, 0, 191, 127);
  scene.pushMask(&scene, viewport, BorderStyle(16, 0));

  const BlitPlan plan = scene.plan(viewport, viewport, 0, -8);

  EXPECT_EQ(Box(5, 13, 186, 122), plan.source);
  EXPECT_EQ(Box(5, 5, 186, 114), plan.destination);
  EXPECT_EQ(20020, plan.area());
  ExpectPlanSafe(scene, plan, viewport, viewport, 0, -8);
}

// Verifies positive, negative, axial, and diagonal translations remain inside
// both rounded endpoints.
TEST(BlitPlanTest, ProvesBothEndpointsForEveryTranslationDirection) {
  PlannerScene scene;
  const Box viewport(4, 7, 163, 102);
  scene.pushMask(&scene, viewport, BorderStyle(20, 12, 24, 8, SmallNumber(1)));
  constexpr std::array<std::array<int16_t, 2>, 8> kDeltas = {{{{8, 0}},
                                                              {{-8, 0}},
                                                              {{0, 7}},
                                                              {{0, -7}},
                                                              {{6, 5}},
                                                              {{-6, 5}},
                                                              {{6, -5}},
                                                              {{-6, -5}}}};

  for (const std::array<int16_t, 2>& delta : kDeltas) {
    SCOPED_TRACE(testing::Message() << delta[0] << ", " << delta[1]);
    const BlitPlan plan = scene.plan(viewport, viewport, delta[0], delta[1]);
    ExpectPlanSafe(scene, plan, viewport, viewport, delta[0], delta[1]);
  }
}

// Verifies nested rounded geometry and outlines are intersected at both the
// source and destination rather than accepting either endpoint alone.
TEST(BlitPlanTest, IntersectsNestedOutlinedMasks) {
  PlannerScene scene;
  const Box viewport(0, 0, 199, 139);
  int outer;
  int inner;
  scene.pushMask(&outer, viewport, BorderStyle(28, SmallNumber(2)));
  scene.pushMask(&inner, Box(9, 5, 190, 132),
                 BorderStyle(12, 24, 18, 30, SmallNumber(1)));

  const BlitPlan plan = scene.plan(viewport, viewport, -9, 6);

  ExpectPlanSafe(scene, plan, viewport, viewport, -9, 6);
}

// Verifies current foreground and its translated source footprint are both
// removed, including ordinary rectangles, translucent overlay extents, and
// conservative masked-exclusion bounds.
TEST(BlitPlanTest, SubtractsEveryForegroundKindAtBothEndpoints) {
  PlannerScene scene;
  const Box viewport(0, 0, 191, 127);
  scene.addBoxRestriction(Box(18, 28, 37, 94));
  auto overlay = roo_display::MakeRasterizable(
      Box(142, 16, 168, 55),
      [](int16_t, int16_t) -> Color { return Color(0x80FF0000); });
  scene.addOverlayRestriction(overlay, overlay.extents());
  int owner;
  scene.pushMask(&owner, viewport, BorderStyle(16, 0));
  scene.addBoxRestriction(Box(72, 48, 124, 92));

  const BlitPlan plan = scene.plan(viewport, viewport, 7, -6);

  ExpectPlanSafe(scene, plan, viewport, viewport, 7, -6);
  ExpectPlanAvoidsMaskedBounds(scene, plan, 7, -6);
}

// Verifies previous-frame validity is supplied only by the source certificate;
// the planner never expands it while fitting current rounded geometry.
TEST(BlitPlanTest, PreservesTheSourceCertificate) {
  PlannerScene scene;
  const Box viewport(0, 0, 159, 95);
  const Box source_certificate(24, 11, 137, 86);
  int owner;
  scene.pushMask(&owner, viewport, BorderStyle(18, 0));

  const BlitPlan plan = scene.plan(source_certificate, viewport, -5, 9);

  ExpectPlanSafe(scene, plan, source_certificate, viewport, -5, 9);
  EXPECT_TRUE(source_certificate.contains(plan.source));
}

// Verifies deep foreground fragmentation reaches the constant-stack fallback
// while the returned rectangle still passes the independent pixel oracle.
TEST(BlitPlanTest, BoundedSearchRemainsConservative) {
  PlannerScene scene;
  const Box viewport(0, 0, 191, 127);
  for (int i = 0; i < 24; ++i) {
    const int16_t x = 12 + (i * 7) % 164;
    const int16_t y = 10 + (i * 19) % 104;
    scene.addBoxRestriction(Box(x, y, x + 2, y + 5));
  }
  int owner;
  scene.pushMask(&owner, viewport, BorderStyle(16, 0));

  const BlitPlan plan = scene.plan(viewport, viewport, 4, -3);

  ExpectPlanSafe(scene, plan, viewport, viewport, 4, -3);
}

// Verifies absent history, non-overlapping history, and a zero translation
// produce no speculative copy plan.
TEST(BlitPlanTest, RejectsStaleOrInapplicableInputs) {
  PlannerScene scene;
  const Box viewport(0, 0, 95, 63);
  int owner;
  scene.pushMask(&owner, viewport, BorderStyle(12, 0));

  EXPECT_TRUE(scene.plan(Box(0, 0, -1, -1), viewport, 4, 0).empty());
  EXPECT_TRUE(scene.plan(Box(120, 0, 160, 63), viewport, 4, 0).empty());
  EXPECT_TRUE(scene.plan(viewport, viewport, 0, 0).empty());
  EXPECT_TRUE(scene.plan(viewport, viewport, 96, 0).empty());
}

}  // namespace
}  // namespace roo_windows
