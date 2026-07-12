#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/switch/badged_switch.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

Rect InsetRect(const Rect& bounds, int16_t inset) {
  if (bounds.empty() || bounds.width() <= 2 * inset ||
      bounds.height() <= 2 * inset) {
    return EmptyRect();
  }
  return Rect(bounds.xMin() + inset, bounds.yMin() + inset,
              bounds.xMax() - inset, bounds.yMax() - inset);
}

Insets InsetsFromEnvelope(const Rect& logical_bounds, const Rect& envelope) {
  if (logical_bounds.empty() || envelope.empty()) {
    return Insets::Zero();
  }
  Rect combined = Rect::Extent(logical_bounds, envelope);
  return Insets(combined.xMin() - logical_bounds.xMin(),
                combined.yMin() - logical_bounds.yMin(),
                logical_bounds.xMax() - combined.xMax(),
                logical_bounds.yMax() - combined.yMax());
}

class SolidBackdrop : public BasicSurfaceWidget {
 public:
  SolidBackdrop(ApplicationContext& context, Color color, Dimensions dims)
      : BasicSurfaceWidget(context), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class BadgeAnchorHost : public Widget {
 public:
  static constexpr int16_t kHostSize = Scaled(40);
  static constexpr int16_t kAnchorSize = Scaled(24);

  explicit BadgeAnchorHost(ApplicationContext& context, bool unclipped = false)
      : Widget(context) {
    if (unclipped) {
      setParentClipMode(ParentClipMode::kUnclipped);
    }
  }

  void setBadgeDot() {
    badge_.setDot();
    requestLayout();
    setDirty();
  }

  void setBadgeText(roo::string_view text) {
    badge_.setText(text);
    requestLayout();
    setDirty();
  }

  void setBadgeValue(unsigned int value) {
    badge_.setValue(value);
    requestLayout();
    setDirty();
  }

  void setBadgePlacement(BadgePlacement placement) {
    placement_ = placement;
    requestLayout();
    setDirty();
  }

  Insets getInkInsets() const override {
    if (!badge_.visible()) return Insets::Zero();
    return InsetsFromEnvelope(
        bounds(), Badge::ConservativeBounds(anchorBounds(), placement_,
                                            badge_.mode() == BadgeMode::kText));
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(kHostSize, kHostSize);
  }

  void paint(PaintContext& ctx) const override {
    // Settle the badge first so the icon square repaints only the lower-z
    // pixels behind it.
    badge_.paint(ctx, theme());
    Rect anchor = anchorBounds();
    ctx.fillRect(anchor, theme().material3Theme().color.primaryContainer);

    Rect glyph = InsetRect(anchor, Scaled(6));
    if (!glyph.empty()) {
      ctx.fillRect(glyph, theme().material3Theme().color.onPrimaryContainer);
    }
  }

 protected:
  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    (void)rect;
    badge_.layout(anchorBounds(), placement_);
  }

  Rect getDirectPaintExclusionBounds() const override { return anchorBounds(); }

 private:
  Rect anchorBounds() const {
    Rect local = bounds();
    if (local.empty()) return EmptyRect();
    int16_t left = local.xMin() + (local.width() - kAnchorSize) / 2;
    int16_t top = local.yMin() + (local.height() - kAnchorSize) / 2;
    return Rect(left, top, left + kAnchorSize - 1, top + kAnchorSize - 1);
  }

  Badge badge_;
  BadgePlacement placement_;
};

class Material3BadgeGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 232;
  static constexpr int16_t kHeight = 72;
  static constexpr Color kBackdrop = Color(0xFFF3EFE7);

  Material3BadgeGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderBadgeStatesRow() {
    Application app(&env_, display_);
    AddBackdrop(app, Dimensions(184, 56));

    BadgeAnchorHost* dot = AddBadgeAnchor(app, 16, 16, false);
    dot->setBadgeDot();

    BadgeAnchorHost* text = AddBadgeAnchor(app, 72, 16, false);
    text->setBadgeText("NEW");

    BadgeAnchorHost* value = AddBadgeAnchor(app, 128, 16, false);
    value->setBadgeValue(1000);
    value->setBadgePlacement({.gravity = BadgeGravity::kTopStart});

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 184, 56);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderOverflowClippingRow() {
    Application app(&env_, display_);
    AddBackdrop(app, Dimensions(184, 60));

    BadgePlacement overhang;
    overhang.gravity = BadgeGravity::kTopEnd;
    overhang.horizontal_offset = -12;
    overhang.vertical_offset = -12;

    BadgeAnchorHost* clipped = AddBadgeAnchor(app, 24, 18, false);
    clipped->setBadgeValue(1000);
    clipped->setBadgePlacement(overhang);

    BadgeAnchorHost* unclipped = AddBadgeAnchor(app, 104, 18, true);
    unclipped->setBadgeValue(1000);
    unclipped->setBadgePlacement(overhang);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 184, 60);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderBadgedSwitchRow() {
    Application app(&env_, display_);
    AddBackdrop(app, Dimensions(228, 64));

    BadgedSwitch* dot = AddBadgedSwitch(app, 12, 16, false);
    dot->setBadgeDot();

    BadgedSwitch* text = AddBadgedSwitch(app, 88, 16, true);
    text->setBadgeText("NEW");
    text->setBadgePlacement({.gravity = BadgeGravity::kTopStart});

    BadgedSwitch* value = AddBadgedSwitch(app, 164, 16, false);
    value->setBadgeValue(1000);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, 228, 64);
  }

 private:
  void AddBackdrop(Application& app, Dimensions dims) {
    auto backdrop =
        std::make_unique<SolidBackdrop>(app.context(), kBackdrop, dims);
    app.add(std::move(backdrop),
            roo_display::Box(0, 0, dims.width() - 1, dims.height() - 1));
  }

  BadgeAnchorHost* AddBadgeAnchor(Application& app, int16_t x, int16_t y,
                                  bool unclipped) {
    auto host = std::make_unique<BadgeAnchorHost>(app.context(), unclipped);
    BadgeAnchorHost* ptr = host.get();
    app.add(std::move(host),
            roo_display::Box(x, y, x + BadgeAnchorHost::kHostSize - 1,
                             y + BadgeAnchorHost::kHostSize - 1));
    return ptr;
  }

  BadgedSwitch* AddBadgedSwitch(Application& app, int16_t x, int16_t y,
                                bool on) {
    auto sw = std::make_unique<BadgedSwitch>(app.context(), on);
    BadgedSwitch* ptr = sw.get();
    app.add(std::move(sw),
            roo_display::Box(x, y, x + Scaled(52) - 1, y + Scaled(32) - 1));
    return ptr;
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Verifies the badge helper's locked-down dot, text, and capped-number
// appearance on the same square anchor row.
TEST_F(Material3BadgeGoldenTest, BadgeStatesRowGolden) {
  auto image = RenderBadgeStatesRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_badge/badge_states_row.ppm",
      "material3_badge_states_row"));
}

// Verifies that the same overhanging badge is clipped away by default and
// remains visible once the host flips to ParentClipMode::kUnclipped.
TEST_F(Material3BadgeGoldenTest, OverflowClippingRowGolden) {
  auto image = RenderOverflowClippingRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_badge/overflow_clipping_row.ppm",
      "material3_badge_overflow_clipping_row"));
}

// Verifies a locked-down host-integration render reference for the badge-aware
// Material 3 switch family.
TEST_F(Material3BadgeGoldenTest, BadgedSwitchRowGolden) {
  auto image = RenderBadgedSwitchRow();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_badge/badged_switch_row.ppm",
      "material3_badge_badged_switch_row"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
