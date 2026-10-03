#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/rounded_clip.h"
#include "roo_windows/material3/menu/menu_surface.h"

namespace roo_windows {
namespace {

using roo_display::AlphaBlend;
using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
namespace color = roo_display::color;

constexpr int kWidth = 96;
constexpr int kHeight = 72;
constexpr Color kPanel(0xFFECE4DB);
constexpr Color kRow(0xFF3867C7);
constexpr Color kRed(0xFFCF3E32);
constexpr Color kBlue(0xFF246FCE);
constexpr Color kGreen(0xFF2E8B57);
constexpr Color kTranslucentOrange(0x809C4E18);

Color Backdrop(int x, int y) {
  return ((x / 5 + y / 5) & 1) == 0 ? Color(0xFF9A7F62) : Color(0xFF314B43);
}

// Counts actual device writes across every output entry point, after filters.
class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* data)
      : OffscreenDevice(kWidth, kHeight, data, Argb8888()) {}

  void reset() { writes.fill(0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
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

class Pattern : public Widget {
 public:
  using Widget::Widget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(kWidth, kHeight);
  }
  void paint(PaintContext& ctx) const override {
    for (int y = 0; y < height(); y += 5) {
      for (int x = 0; x < width(); x += 5) {
        ctx.fillRect(Rect(x, y, std::min(x + 4, width() - 1),
                          std::min(y + 4, int(height()) - 1)),
                     Backdrop(x, y));
      }
    }
  }
};

class RoundedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  bool clipsChildrenToRoundedBounds() const override { return true; }
  Color background() const override { return kPanel; }
  BorderStyle getBorderStyle() const override { return BorderStyle(16, 0); }
};

class SelectedRow : public SurfaceWidget {
 public:
  using SurfaceWidget::moveTo;
  using SurfaceWidget::SurfaceWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(64, 24);
  }
  Color background() const override { return kRow; }
  bool isClickable() const override { return true; }
  BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }
  void paint(PaintContext& ctx) const override {
    ++paint_count;
    ctx.clear();
    if (delay_ms != 0) delay(delay_ms);
  }
  mutable int paint_count = 0;
  int delay_ms = 0;
};

class PaintBlock : public Widget {
 public:
  PaintBlock(ApplicationContext& context, Color color,
             bool deferred_overlay = false)
      : Widget(context), color_(color), deferred_overlay_(deferred_overlay) {}

  void paint(PaintContext& ctx) const override {
    ++paint_count;
    if (deferred_overlay_) {
      ctx.addOverlayShape(roo_display::SmoothFilledRoundRect(
          0, 0, width() - 1, height() - 1, 0, color_));
    } else {
      ctx.fillRect(bounds(), color_);
    }
    if (delay_ms != 0) delay(delay_ms);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  void setColor(Color color) {
    color_ = color;
    invalidateInterior();
  }

  mutable int paint_count = 0;
  int delay_ms = 0;

 protected:
  Rect getDirectPaintExclusionBounds() const override {
    return deferred_overlay_ ? Rect() : bounds();
  }

 private:
  Color color_;
  bool deferred_overlay_;
};

class SloppyPaintBlock : public PaintBlock {
 public:
  using PaintBlock::PaintBlock;

  Rect getSloppyTouchParentBounds() const override {
    return Rect(parent_bounds().xMin() - 8, parent_bounds().yMin(),
                parent_bounds().xMax(), parent_bounds().yMax());
  }
};

class FixedRoundedPanel : public RoundedPanel {
 public:
  using RoundedPanel::RoundedPanel;

  bool mayHaveUnclippedChildren() const override { return false; }
};

class PlainGroupedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;

  Color background() const override { return kPanel; }
};

class FixedPlainPanel : public PlainGroupedPanel {
 public:
  using PlainGroupedPanel::PlainGroupedPanel;

  bool mayHaveUnclippedChildren() const override { return false; }
};

class CountingRoundedPanel : public RoundedPanel {
 public:
  using RoundedPanel::RoundedPanel;

  bool mayHaveUnclippedChildren() const override {
    ++capability_queries;
    return true;
  }

  mutable int capability_queries = 0;
};

class RoundedClipTest : public testing::Test {
 protected:
  RoundedClipTest()
      : device_(data_.data()),
        display_(device_),
        env_(scheduler_),
        app_(&env_, display_) {}

  void SetUp() override {
    ASSERT_TRUE(app_.refresh());
    auto backdrop = std::make_unique<Pattern>(app_.context());
    backdrop_ = backdrop.get();
    app_.add(std::move(backdrop), Box(0, 0, 95, 71));
  }

  Color pixel(int16_t x, int16_t y) const {
    Color result;
    device_.raster().readColors(&x, &y, 1, &result);
    return result;
  }

  void expectColor(Color actual, Color expected, int x, int y) {
    EXPECT_NEAR(actual.r(), expected.r(), 2) << x << ',' << y;
    EXPECT_NEAR(actual.g(), expected.g(), 2) << x << ',' << y;
    EXPECT_NEAR(actual.b(), expected.b(), 2) << x << ',' << y;
    EXPECT_EQ(actual.a(), expected.a()) << x << ',' << y;
  }

  void saveFrame(int position) {
    const char* dir = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR");
    if (dir == nullptr) return;
    const std::string path = std::string(dir) + "/rounded_scroll_" +
                             std::to_string(position) + ".ppm";
    FILE* file = std::fopen(path.c_str(), "wb");
    ASSERT_NE(file, nullptr);
    std::fprintf(file, "P6\n%d %d\n255\n", kWidth, kHeight);
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        const Color c = pixel(x, y);
        const unsigned char rgb[] = {c.r(), c.g(), c.b()};
        std::fwrite(rgb, 1, 3, file);
      }
    }
    std::fclose(file);
  }

  void expectSingleWrite() {
    for (size_t i = 0; i < device_.writes.size(); ++i) {
      EXPECT_LE(device_.writes[i], 1) << i % kWidth << ',' << i / kWidth;
    }
  }

  std::array<roo::byte, kWidth * kHeight * 4> data_{};
  RecordingDevice device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
  Pattern* backdrop_ = nullptr;
};

// Verifies all pixels against independently composed child/parent decorations,
// while counting one child paint and at most one device write per pixel.
TEST_F(RoundedClipTest, ScrollingRoundedSelectionOverPattern) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  RoundedPanel* panel_ptr = panel.get();
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* row_ptr = row.get();
  panel_ptr->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  for (int position : {-7, 0, 20, 32, -3}) {
    row_ptr->moveTo(Rect(0, position, 63, position + 23));
    const int before = row_ptr->paint_count;
    device_.reset();
    ASSERT_TRUE(app_.refresh());
    EXPECT_EQ(row_ptr->paint_count, before + 1);
    Decoration child(Box(16, 12 + position, 79, 35 + position), 0,
                     OverlaySpec(), nullptr, kRow, {8, 8, 8, 8}, 0, kRow);
    Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                      {16, 16, 16, 16}, 0, kPanel);
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        Color c;
        child.readColors(&x, &y, 1, &c);
        const Color content = AlphaBlend(kPanel, c);
        const Color expected =
            AlphaBlend(Backdrop(x, y), parent.readWithContent(x, y, content));
        expectColor(pixel(x, y), expected, x, y);
      }
    }
    expectSingleWrite();
    saveFrame(position);
  }
}

// Verifies the production menu viewport exposes selected rows at its top
// curve after scrolling, with fractional coverage and one final device write.
TEST_F(RoundedClipTest, MenuScrollReachesAntialiasedPanelEdge) {
  using namespace material3;
  auto panel = std::make_unique<material3::internal::MenuPanel>(app_.context());
  auto* menu_panel = panel.get();
  auto group = std::make_unique<MenuGroup>(app_.context());
  MenuGroup* rows = group.get();
  StandardMenuItemInit init;
  init.flags =
      StandardMenuItemFlags::kSelectable | StandardMenuItemFlags::kSelected;
  auto row = std::make_unique<MenuRow<StandardMenuItem>>(app_.context(), init);
  MenuEntry* selected = row.get();
  group->add(std::move(row));
  panel->addGroup(std::move(group));
  auto second_group = std::make_unique<MenuGroup>(app_.context());
  second_group->add(
      std::make_unique<MenuRow<StandardMenuItem>>(app_.context()));
  panel->addGroup(std::move(second_group));
  MenuPolicy policy;
  policy.separator_mode = MenuSeparatorMode::kDivider;
  panel->setPolicy(policy);
  const Box bounds(16, 12, 79, 55);
  app_.add(std::move(panel), bounds);
  ASSERT_TRUE(app_.refresh());
  auto* viewport = dynamic_cast<material3::internal::MenuViewport*>(
      rows->parent()->parent());
  ASSERT_NE(viewport, nullptr);
  const Color background = menu_panel->background();
  const BorderStyle border = menu_panel->getBorderStyle();
  Decoration parent(bounds, menu_panel->getElevation(), OverlaySpec(), nullptr,
                    background, border.corner_radii(), 0, background);
  for (int offset : {0, 12, 20, 0}) {
    SCOPED_TRACE(offset);
    viewport->scrollTo(0, -offset);
    device_.reset();
    ASSERT_TRUE(app_.refresh());
    const int top = bounds.yMin() + Scaled(4) - offset;
    const int left = bounds.xMin() + Scaled(4);
    Decoration child(Box(left, top, left + selected->width() - 1,
                         top + selected->height() - 1),
                     0, OverlaySpec(), nullptr, selected->background(),
                     selected->getBorderStyle().corner_radii(), 0,
                     selected->background());
    int selected_boundary_pixels = 0;
    // This strip used to be a stationary gutter. Stay left of text/checkmarks.
    for (int16_t y = bounds.yMin(); y < bounds.yMin() + Scaled(4); ++y) {
      for (int16_t x = bounds.xMin(); x < bounds.xMin() + Scaled(14); ++x) {
        Color c;
        child.readColors(&x, &y, 1, &c);
        const Color expected =
            AlphaBlend(Backdrop(x, y),
                       parent.readWithContent(x, y, AlphaBlend(background, c)));
        expectColor(pixel(x, y), expected, x, y);
        const uint8_t coverage = roo_windows::internal::RoundedFillCoverage(
            bounds, border.corner_radii(), 0, x, y);
        if (coverage > 0 && coverage < 255 && c.a() > 0) {
          ++selected_boundary_pixels;
        }
      }
    }
    if (offset > 0) {
      EXPECT_GT(selected_boundary_pixels, 0);
    }
    if (offset == 20) {
      const int y = top + selected->height();
      const Color divider =
          app_.context().theme().material3Theme().color.resolve(
              ColorToken::kOutlineVariant);
      expectColor(pixel(43, y), divider, 43, y);
      expectColor(pixel(43, y - 1), selected->background(), 43, y - 1);
    }
    expectSingleWrite();
  }
  viewport->scrollToBottom();
  EXPECT_EQ(viewport->height() - Scaled(4),
            viewport->contents()->offsetTop() +
                menu_panel->groupAt(1).parent_bounds().yMax() + 1);
}

// Verifies a clean clipped foreground is reconstructed once when its backdrop
// changes, rather than losing the selected color at the antialiased edge.
TEST_F(RoundedClipTest, BackdropOnlyDamageReconstructsBoundaryOnce) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* selected = row.get();
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  ASSERT_TRUE(app_.refresh());
  std::array<Color, kWidth * kHeight> expected;
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) expected[y * kWidth + x] = pixel(x, y);
  }
  backdrop_->invalidateInterior();
  device_.reset();
  const int before = selected->paint_count;
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(selected->paint_count, before + 1);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      expectColor(pixel(x, y), expected[y * kWidth + x], x, y);
    }
  }
  expectSingleWrite();
}

// Verifies a timeout retains captured colors and finished child output without
// visiting the child again or writing its settled interior twice.
TEST_F(RoundedClipTest, ContinuationRetainsBoundaryAndChildProgress) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* selected = row.get();
  selected->delay_ms = 40;
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  device_.reset();
  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  EXPECT_EQ(selected->paint_count, 1);
  selected->delay_ms = 0;
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(selected->paint_count, 1);
  expectSingleWrite();
  Decoration child(Box(16, 5, 79, 28), 0, OverlaySpec(), nullptr, kRow,
                   {8, 8, 8, 8}, 0, kRow);
  Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c;
      child.readColors(&x, &y, 1, &c);
      expectColor(pixel(x, y),
                  AlphaBlend(Backdrop(x, y), parent.readWithContent(
                                                 x, y, AlphaBlend(kPanel, c))),
                  x, y);
    }
  }
}

// Verifies a mutation between attempts discards stale edge contributions and
// reconstructs the new scene, with single writes during the replacement frame.
TEST_F(RoundedClipTest, MutationDuringContinuationRestartsBoundary) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* selected = row.get();
  selected->delay_ms = 40;
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  ASSERT_EQ(selected->paint_count, 1);
  selected->delay_ms = 0;
  selected->moveTo(Rect(0, 20, 63, 43));
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(selected->paint_count, 2);
  expectSingleWrite();
  Decoration child(Box(16, 32, 79, 55), 0, OverlaySpec(), nullptr, kRow,
                   {8, 8, 8, 8}, 0, kRow);
  Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c;
      child.readColors(&x, &y, 1, &c);
      expectColor(pixel(x, y),
                  AlphaBlend(Backdrop(x, y), parent.readWithContent(
                                                 x, y, AlphaBlend(kPanel, c))),
                  x, y);
    }
  }
}

// Verifies unclipped children form the foreground group regardless of raw
// insertion order, escape the parent curve, and restore lower clipped content.
TEST_F(RoundedClipTest, UnclippedForegroundRestoresAndChangesGroups) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto foreground = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* foreground_ptr = foreground.get();
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(foreground), Rect(-8, -8, 31, 31));
  auto clipped = std::make_unique<PaintBlock>(app_.context(), kRed);
  panel->add(std::move(clipped), Rect(0, 0, 63, 43));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(32, 28), kBlue);
  EXPECT_EQ(pixel(16, 12), kBlue);
  EXPECT_EQ(pixel(10, 8), kBlue);
  expectSingleWrite();

  foreground_ptr->setVisibility(Visibility::kInvisible);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(32, 28), kRed);
  EXPECT_EQ(pixel(10, 8), Backdrop(10, 8));
  expectSingleWrite();

  foreground_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(app_.refresh());
  foreground_ptr->setParentClipMode(ParentClipMode::kClipped);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(32, 28), kRed);
  EXPECT_EQ(pixel(10, 8), Backdrop(10, 8));
  expectSingleWrite();
}

// Verifies a translucent unclipped overlay composes outside the parent's
// retained boundary buffer and remains above a higher-index clipped sibling.
TEST_F(RoundedClipTest, TranslucentUnclippedForegroundComposesOutsideMask) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto foreground = std::make_unique<PaintBlock>(
      app_.context(), kTranslucentOrange, /*deferred_overlay=*/true);
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(foreground), Rect(-8, -8, 31, 31));
  panel->add(std::make_unique<PaintBlock>(app_.context(), kRed),
             Rect(0, 0, 63, 43));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  device_.reset();
  ASSERT_TRUE(app_.refresh());
  Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  const Color inside = parent.readWithContent(32, 28, kRed);
  expectColor(pixel(32, 28), AlphaBlend(inside, kTranslucentOrange), 32, 28);
  expectColor(pixel(10, 8), AlphaBlend(Backdrop(10, 8), kTranslucentOrange), 10,
              8);
  expectSingleWrite();
}

// Verifies an unclipped descendant bypasses only its immediate rounded parent;
// the enclosing rounded ancestor still masks the escaped pixels.
TEST_F(RoundedClipTest, UnclippedChildRetainsRoundedAncestorMask) {
  auto outer = std::make_unique<RoundedPanel>(app_.context());
  auto inner = std::make_unique<RoundedPanel>(app_.context());
  auto foreground = std::make_unique<PaintBlock>(app_.context(), kBlue);
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  inner->add(std::move(foreground), Rect(-12, -12, 35, 35));
  outer->add(std::move(inner), Rect(12, 10, 51, 37));
  app_.add(std::move(outer), Box(16, 12, 79, 55));

  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(24, 20), kBlue);
  EXPECT_EQ(pixel(16, 12), Backdrop(16, 12));
  expectSingleWrite();
}

// Verifies exact targets retain precedence over foreground-group sloppy hits,
// while overlapping exact targets follow the visual child-group order.
TEST_F(RoundedClipTest, TouchSearchMatchesGroupedVisualOrder) {
  auto panel = std::make_unique<PlainGroupedPanel>(app_.context());
  PlainGroupedPanel* panel_ptr = panel.get();
  auto foreground = std::make_unique<SloppyPaintBlock>(app_.context(), kBlue);
  SloppyPaintBlock* foreground_ptr = foreground.get();
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(foreground), Rect(12, 0, 31, 19));
  auto clipped = std::make_unique<PaintBlock>(app_.context(), kRed);
  PaintBlock* clipped_ptr = clipped.get();
  panel->add(std::move(clipped), Rect(4, 0, 23, 19));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  std::vector<Widget*> path;
  ASSERT_TRUE(panel_ptr->fillTouchTargetPath(16, 8, path));
  EXPECT_EQ(foreground_ptr, path.back());

  path.clear();
  ASSERT_TRUE(panel_ptr->fillTouchTargetPath(8, 8, path));
  EXPECT_EQ(clipped_ptr, path.back());
}

// Verifies a deadline after an unclipped child retains the foreground cursor;
// the continuation does not repaint that completed child before clipped work.
TEST_F(RoundedClipTest, ContinuationRetainsUnclippedGroupProgress) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto lower_foreground = std::make_unique<PaintBlock>(app_.context(), kGreen);
  PaintBlock* lower_foreground_ptr = lower_foreground.get();
  lower_foreground->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(lower_foreground), Rect(-8, -8, 31, 31));
  auto clipped = std::make_unique<PaintBlock>(app_.context(), kRed);
  PaintBlock* clipped_ptr = clipped.get();
  panel->add(std::move(clipped), Rect(0, 0, 63, 43));
  auto foreground = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* foreground_ptr = foreground.get();
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  foreground->delay_ms = 40;
  panel->add(std::move(foreground), Rect(-8, -8, 31, 31));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  device_.reset();
  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  EXPECT_EQ(foreground_ptr->paint_count, 1);
  EXPECT_EQ(lower_foreground_ptr->paint_count, 0);
  EXPECT_EQ(clipped_ptr->paint_count, 0);
  foreground_ptr->delay_ms = 0;
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(foreground_ptr->paint_count, 1);
  EXPECT_EQ(lower_foreground_ptr->paint_count, 1);
  EXPECT_EQ(clipped_ptr->paint_count, 1);
  EXPECT_EQ(pixel(32, 28), kBlue);
  expectSingleWrite();
}

// Verifies clipped-pass continuation retains its descending cursor while the
// rounded record remains fresh for clean lower clipped contributors.
TEST_F(RoundedClipTest, ContinuationRetainsClippedGroupProgress) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto lower = std::make_unique<PaintBlock>(app_.context(), kRed);
  PaintBlock* lower_ptr = lower.get();
  panel->add(std::move(lower), Rect(0, 0, 63, 43));
  auto higher = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* higher_ptr = higher.get();
  higher->delay_ms = 40;
  panel->add(std::move(higher), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  device_.reset();
  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  EXPECT_EQ(higher_ptr->paint_count, 1);
  EXPECT_EQ(lower_ptr->paint_count, 0);
  higher_ptr->delay_ms = 0;
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(higher_ptr->paint_count, 1);
  EXPECT_EQ(lower_ptr->paint_count, 1);
  EXPECT_EQ(pixel(32, 28), kBlue);
  expectSingleWrite();
}

// Verifies mutation while the unclipped cursor is retained restarts the whole
// rounded scene and discards the completed foreground child's stale color.
TEST_F(RoundedClipTest, MutationDuringUnclippedContinuationRestartsGroups) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto lower = std::make_unique<PaintBlock>(app_.context(), kRed);
  lower->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(lower), Rect(32, 0, 63, 31));
  auto foreground = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* foreground_ptr = foreground.get();
  foreground->setParentClipMode(ParentClipMode::kUnclipped);
  foreground->delay_ms = 40;
  panel->add(std::move(foreground), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  ASSERT_EQ(foreground_ptr->paint_count, 1);
  foreground_ptr->delay_ms = 0;
  foreground_ptr->setColor(kGreen);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(foreground_ptr->paint_count, 2);
  EXPECT_EQ(pixel(32, 28), kGreen);
  expectSingleWrite();
}

// Verifies mutation while the clipped cursor is retained restarts capture and
// replaces boundary/interior output from the completed clipped child.
TEST_F(RoundedClipTest, MutationDuringClippedContinuationRestartsGroups) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  panel->add(std::make_unique<PaintBlock>(app_.context(), kRed),
             Rect(32, 0, 63, 31));
  auto higher = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* higher_ptr = higher.get();
  higher->delay_ms = 40;
  panel->add(std::move(higher), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(16, 12, 79, 55));

  EXPECT_FALSE(app_.refresh(roo_time::Uptime::Now() + roo_time::Millis(20)));
  ASSERT_EQ(higher_ptr->paint_count, 1);
  higher_ptr->delay_ms = 0;
  higher_ptr->setColor(kGreen);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(higher_ptr->paint_count, 2);
  EXPECT_EQ(pixel(32, 28), kGreen);
  expectSingleWrite();
}

// Verifies a false capability uses the all-clipped scan while matching the
// pixels and one-paint behavior of the conservatively true control.
TEST_F(RoundedClipTest, GuaranteedClippedFastPathMatchesGroupedControl) {
  auto fixed = std::make_unique<FixedRoundedPanel>(app_.context());
  auto fixed_child = std::make_unique<PaintBlock>(app_.context(), kRed);
  PaintBlock* fixed_child_ptr = fixed_child.get();
  fixed->add(std::move(fixed_child), Rect(0, 0, 31, 31));
  app_.add(std::move(fixed), Box(8, 20, 39, 51));

  auto grouped = std::make_unique<RoundedPanel>(app_.context());
  auto grouped_child = std::make_unique<PaintBlock>(app_.context(), kRed);
  PaintBlock* grouped_child_ptr = grouped_child.get();
  grouped->add(std::move(grouped_child), Rect(0, 0, 31, 31));
  app_.add(std::move(grouped), Box(48, 20, 79, 51));

  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(fixed_child_ptr->paint_count, 1);
  EXPECT_EQ(grouped_child_ptr->paint_count, 1);
  for (int16_t y = 20; y <= 51; ++y) {
    for (int16_t x = 8; x <= 39; ++x) {
      expectColor(pixel(x, y), pixel(x + 40, y), x, y);
    }
  }
  expectSingleWrite();
}

// Verifies one uninterrupted rounded child traversal evaluates the virtual
// capability once rather than rescanning or querying once per group.
TEST_F(RoundedClipTest, CapabilityIsQueriedOncePerTraversal) {
  auto panel = std::make_unique<CountingRoundedPanel>(app_.context());
  CountingRoundedPanel* panel_ptr = panel.get();
  panel->add(std::make_unique<PaintBlock>(app_.context(), kRed),
             Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(24, 20, 55, 51));
  panel_ptr->capability_queries = 0;

  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(panel_ptr->capability_queries, 1);
}

// Verifies grouping is independent of rounded clipping and preserves descending
// collection order inside the unclipped foreground group.
TEST_F(RoundedClipTest, NonRoundedContainerStillGroupsChildren) {
  auto panel = std::make_unique<PlainGroupedPanel>(app_.context());
  auto lower_unclipped = std::make_unique<PaintBlock>(app_.context(), kBlue);
  PaintBlock* lower_ptr = lower_unclipped.get();
  lower_unclipped->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(lower_unclipped), Rect(0, 0, 31, 31));
  panel->add(std::make_unique<PaintBlock>(app_.context(), kRed),
             Rect(0, 0, 31, 31));
  auto higher_unclipped = std::make_unique<PaintBlock>(app_.context(), kGreen);
  PaintBlock* higher_ptr = higher_unclipped.get();
  higher_unclipped->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(higher_unclipped), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(24, 20, 55, 51));

  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(32, 28), kGreen);
  higher_ptr->setVisibility(Visibility::kInvisible);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(32, 28), kBlue);
  EXPECT_EQ(lower_ptr->paint_count, 2);
  expectSingleWrite();
}

#ifndef NDEBUG
// Verifies a false capability is a checked authoring promise rather than a
// request to silently clip a currently-unclipped direct child.
TEST_F(RoundedClipTest, InvalidFalseCapabilityFailsDebugContract) {
  auto panel = std::make_unique<FixedRoundedPanel>(app_.context());
  auto child = std::make_unique<PaintBlock>(app_.context(), kBlue);
  child->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(child), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(24, 20, 55, 51));

  EXPECT_DEATH(app_.refresh(), "");
}

TEST_F(RoundedClipTest, InvalidFalseCapabilityFailsForNonRoundedContainer) {
  auto panel = std::make_unique<FixedPlainPanel>(app_.context());
  auto child = std::make_unique<PaintBlock>(app_.context(), kBlue);
  child->setParentClipMode(ParentClipMode::kUnclipped);
  panel->add(std::move(child), Rect(0, 0, 31, 31));
  app_.add(std::move(panel), Box(24, 20, 55, 51));

  EXPECT_DEATH(app_.refresh(), "");
}
#endif

// Verifies the parent outline and shadow retain their existing coverage while
// selected child pixels reach the fractional inner curve.
TEST_F(RoundedClipTest, OutlineAndShadowAroundCapturedContent) {
  class OutlinedPanel : public RoundedPanel {
   public:
    using RoundedPanel::RoundedPanel;
    BorderStyle getBorderStyle() const override { return BorderStyle(16, 2); }
    Color getOutlineColor() const override { return Color(0xFF9B3A27); }
    uint8_t getElevation() const override { return 2; }
  };
  auto panel = std::make_unique<OutlinedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  expectSingleWrite();
  Decoration child(Box(16, 5, 79, 28), 0, OverlaySpec(), nullptr, kRow,
                   {8, 8, 8, 8}, 0, kRow);
  Decoration parent(Box(16, 12, 79, 55), 2, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 2, Color(0xFF9B3A27));
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c(0);
      if (Box(18, 14, 77, 53).contains(x, y)) {
        child.readColors(&x, &y, 1, &c);
      }
      expectColor(pixel(x, y),
                  AlphaBlend(Backdrop(x, y), parent.readWithContent(
                                                 x, y, AlphaBlend(kPanel, c))),
                  x, y);
    }
  }
}

// Verifies a same-color outline still masks the substituted child colors at
// its inner curve, even though flat Decoration can normally fold it away.
TEST_F(RoundedClipTest, MatchingOutlineKeepsInnerAntialiasing) {
  class OutlinedPanel : public RoundedPanel {
   public:
    using RoundedPanel::RoundedPanel;
    BorderStyle getBorderStyle() const override { return BorderStyle(16, 2); }
    Color getOutlineColor() const override { return kPanel; }
    uint8_t getElevation() const override { return 2; }
  };
  auto panel = std::make_unique<OutlinedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  expectSingleWrite();
  Decoration child(Box(16, 5, 79, 28), 0, OverlaySpec(), nullptr, kRow,
                   {8, 8, 8, 8}, 0, kRow);
  Decoration parent(Box(16, 12, 79, 55), 2, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 2, kPanel, true);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c(0);
      if (Box(18, 14, 77, 53).contains(x, y)) {
        child.readColors(&x, &y, 1, &c);
      }
      expectColor(pixel(x, y),
                  AlphaBlend(Backdrop(x, y), parent.readWithContent(
                                                 x, y, AlphaBlend(kPanel, c))),
                  x, y);
    }
  }
}

// Verifies the write/drop/buffer decision across streamed, sparse and rectangle
// output, with a translucent child overlay composed before the parent mask.
TEST_F(RoundedClipTest, EveryOutputPathAndDeferredOverlay) {
  for (int method = 0; method < 6; ++method) {
    internal::ClipperState state;
    Clipper clipper(state, device_, roo_time::Uptime::Max());
    const Box bounds(16, 12, 79, 55);
    clipper.setBounds(Box(0, 0, 95, 71));
    bool fresh = false;
    internal::RoundedClip& clip =
        clipper.prepareRoundedClip(&state, bounds, BorderStyle(16, 0), fresh);
    clipper.activateRoundedClip(clip);
    internal::RoundedClipOutput output(*clipper.out(), clip);
    roo_display::SmoothShape overlay = roo_display::SmoothFilledRoundRect(
        16, 8, 79, 32, 8, Color(96, 230, 20, 80));
    clipper.addOverlay(&overlay, bounds);
    device_.reset();
    if (method < 2) {
      output.setAddress(16, 12, 79, 55, BlendingMode::kSource);
      if (method == 0) {
        // Odd chunks straddle both scanline and curve-run boundaries.
        std::array<Color, 73> chunk;
        chunk.fill(kRow);
        int remaining = 64 * 44;
        while (remaining > 0) {
          const int count = std::min<int>(chunk.size(), remaining);
          output.write(chunk.data(), count);
          chunk.fill(kRow);
          remaining -= count;
        }
      } else {
        output.fill(kRow, 64 * 44);
      }
    } else if (method < 4) {
      for (int16_t y = 12; y <= 55; ++y) {
        std::array<Color, 64> colors;
        std::array<int16_t, 64> xs;
        std::array<int16_t, 64> ys;
        colors.fill(kRow);
        ys.fill(y);
        for (int i = 0; i < 64; ++i) xs[i] = 16 + i;
        if (method == 2) {
          output.writePixels(BlendingMode::kSource, colors.data(), xs.data(),
                             ys.data(), 64);
        } else {
          output.fillPixels(BlendingMode::kSource, kRow, xs.data(), ys.data(),
                            64);
        }
      }
    } else {
      int16_t x0 = 16;
      int16_t y0 = 12;
      int16_t x1 = 79;
      int16_t y1 = 55;
      Color value = kRow;
      if (method == 4) {
        output.writeRects(BlendingMode::kSource, &value, &x0, &y0, &x1, &y1, 1);
      } else {
        output.fillRects(BlendingMode::kSource, value, &x0, &y0, &x1, &y1, 1);
      }
    }
    clipper.addExclusion(bounds);
    clip.phase = internal::RoundedPaintPhase::kComplete;
    clipper.deactivateRoundedClip();
    clipper.addRoundedDecoration(&state, Box(0, 0, 95, 71), bounds, 0, kPanel,
                                 BorderStyle(16, 0), kPanel);
    for (int16_t y = 0; y < kHeight; ++y) {
      std::array<Color, kWidth> values;
      for (int x = 0; x < kWidth; ++x) values[x] = Backdrop(x, y);
      clipper.out()->setAddress(0, y, kWidth - 1, y, BlendingMode::kSource);
      clipper.out()->write(values.data(), values.size());
    }
    Decoration parent(bounds, 0, OverlaySpec(), nullptr, kPanel,
                      {16, 16, 16, 16}, 0, kPanel);
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        Color over(0);
        if (overlay.extents().contains(x, y)) {
          overlay.readColors(&x, &y, 1, &over);
        }
        const Color expected =
            AlphaBlend(Backdrop(x, y),
                       parent.readWithContent(x, y, AlphaBlend(kRow, over)));
        expectColor(pixel(x, y), expected, x, y);
      }
    }
    expectSingleWrite();
  }
}

// Verifies intersecting ancestor and descendant curves apply each surface mask
// once, while each leaf paints once and higher siblings retain their Z order.
TEST_F(RoundedClipTest, NestedClipsAndHigherSibling) {
  auto outer = std::make_unique<RoundedPanel>(app_.context());
  auto inner = std::make_unique<RoundedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* selected = row.get();
  inner->add(std::move(row), Rect(0, -7, 63, 16));
  outer->add(std::move(inner), Rect(-3, -5, 52, 34));
  app_.add(std::move(outer), Box(16, 12, 79, 55));
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(selected->paint_count, 1);
  expectSingleWrite();
  Decoration child(Box(13, 0, 76, 23), 0, OverlaySpec(), nullptr, kRow,
                   {8, 8, 8, 8}, 0, kRow);
  Decoration middle(Box(13, 7, 68, 46), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c;
      child.readColors(&x, &y, 1, &c);
      const Color middle_color =
          middle.readWithContent(x, y, AlphaBlend(kPanel, c));
      const Color expected = AlphaBlend(
          Backdrop(x, y),
          parent.readWithContent(x, y, AlphaBlend(kPanel, middle_color)));
      expectColor(pixel(x, y), expected, x, y);
    }
  }
  auto higher = std::make_unique<SelectedRow>(app_.context());
  app_.add(std::move(higher), Box(8, 8, 71, 31));
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(pixel(24, 16), kRow);
  expectSingleWrite();
}

// Verifies a descendant's animated ripple affects both directly drawn pixels
// and captured boundary colors, without an extra display write.
TEST_F(RoundedClipTest, ChildPressReachesCapturedBoundary) {
  auto panel = std::make_unique<RoundedPanel>(app_.context());
  auto row = std::make_unique<SelectedRow>(app_.context());
  SelectedRow* selected = row.get();
  panel->add(std::move(row), Rect(0, -7, 63, 16));
  app_.add(std::move(panel), Box(16, 12, 79, 55));
  ASSERT_TRUE(app_.refresh());
  selected->onShowPress(32, 12);
  delay(60);
  device_.reset();
  ASSERT_TRUE(app_.refresh());
  roo_display::Surface surface(device_, 16, 5, Box(16, 5, 79, 28), false, kRow,
                               roo_display::FillMode::kVisible,
                               BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  OverlaySpec spec(*selected, canvas);
  ASSERT_TRUE(spec.has_press_overlay());
  const PressOverlaySpec& ps = spec.press_overlay();
  PressOverlay press(ps.center_x, ps.center_y, ps.radius, ps.color);
  Decoration child(Box(16, 5, 79, 28), 0, spec, &press, kRow, {8, 8, 8, 8}, 0,
                   kRow);
  Decoration parent(Box(16, 12, 79, 55), 0, OverlaySpec(), nullptr, kPanel,
                    {16, 16, 16, 16}, 0, kPanel);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color c;
      child.readColors(&x, &y, 1, &c);
      expectColor(pixel(x, y),
                  AlphaBlend(Backdrop(x, y), parent.readWithContent(
                                                 x, y, AlphaBlend(kPanel, c))),
                  x, y);
    }
  }
  expectSingleWrite();
}

// Verifies row geometry exactly agrees with Decoration, including asymmetry,
// fractional outlines, small bounds, and the last support row at each corner.
TEST(RoundedClipGeometryTest, UsesExactDecorationCoverage) {
  for (int width : {1, 3, 32, 96}) {
    for (int height : {1, 4, 35, 72}) {
      const Box bounds(9, 5, 8 + width, 4 + height);
      for (int outline : {0, 8, 16, 36}) {
        BorderStyle style({4, 16, 8, 12}, SmallNumber::Of16ths(outline));
        const BorderStyle trimmed = style.trim(width, height);
        internal::RoundedClip clip;
        clip.reset(&clip, bounds, style);
        for (int y = bounds.yMin(); y <= bounds.yMax(); ++y) {
          for (int x = bounds.xMin(); x <= bounds.xMax(); ++x) {
            const uint8_t alpha = clip.viewport().contains(x, y)
                                      ? internal::RoundedFillCoverage(
                                            bounds, trimmed.corner_radii(),
                                            trimmed.outline_width(), x, y)
                                      : 0;
            const uint8_t expected = alpha == 0 ? 0 : alpha == 255 ? 255 : 1;
            EXPECT_EQ(clip.coverage(x, y), expected)
                << width << 'x' << height << " outline " << outline << " at "
                << x << ',' << y;
          }
        }
      }
    }
  }
}

// Verifies storage grows with curve length, not panel area, and resetting
// identical geometry retains all sparse array capacity.
TEST(RoundedClipGeometryTest, SparseStorageAndCapacityReuse) {
  for (int radius : {4, 8, 14, 16, 24, 32, 64}) {
    internal::RoundedClip clip;
    const int size = std::max(160, radius * 2 + 2);
    clip.reset(&clip, Box(0, 0, size - 1, size - 1), BorderStyle(radius, 0));
    const size_t capacity = clip.storageBytes();
    const size_t colors = clip.colorBytes();
    std::printf("radius=%d color_bytes=%zu storage_capacity=%zu record=%zu\n",
                radius, colors, capacity, sizeof(clip));
    EXPECT_LT(colors, size_t(32 * radius));
    if (radius == 16) {
      EXPECT_EQ(colors, 320u);
    }
    clip.reset(&clip, Box(0, 0, size - 1, size - 1), BorderStyle(radius, 0));
    EXPECT_EQ(clip.storageBytes(), capacity);
    EXPECT_EQ(clip.colorBytes(), colors);
  }
}

}  // namespace
}  // namespace roo_windows
