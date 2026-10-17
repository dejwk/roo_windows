#include <algorithm>
#include <array>
#include <memory>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/dialog/dialog_scaffold.h"
#include "roo_windows/material3/divider/divider.h"

namespace roo_windows::material3 {
namespace {
using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
constexpr int kWidth = 96;
constexpr int kHeight = 72;
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

class Material3DividerTest : public testing::Test {
 protected:
  std::array<roo::byte, kWidth * kHeight * 4> pixels_{};
  RecordingDevice device_{pixels_.data()};
  roo_display::Display display_{device_};
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment environment_{scheduler_, theme_};
  Application app_{&environment_, display_};
  void SetUp() override { app_.refresh(); }
  ApplicationContext& context() { return app_.context(); }
  Color pixel(int16_t x, int16_t y) {
    Color result;
    device_.raster().readColors(&x, &y, 1, &result);
    return result;
  }
  void refresh() {
    device_.reset();
    app_.refresh();
    EXPECT_LE(*std::max_element(device_.writes.begin(), device_.writes.end()),
              1);
  }
};

// Verifies both orientations reserve only their thickness, stretch along the
// other axis, and carry no extra instance state or spacing, including at 75%.
TEST_F(Material3DividerTest, MeasurementAndDefaultHooks) {
  static_assert(sizeof(HorizontalDivider) == sizeof(Widget));
  static_assert(sizeof(VerticalDivider) == sizeof(Widget));
  HorizontalDivider horizontal(context());
  VerticalDivider vertical(context());
  const int thickness = std::max(1, Scaled(1));
  EXPECT_EQ(thickness, horizontal.dividerThickness());
  EXPECT_EQ(thickness, vertical.dividerThickness());
  EXPECT_EQ(0, horizontal.getSuggestedMinimumDimensions().width());
  EXPECT_EQ(thickness, horizontal.getSuggestedMinimumDimensions().height());
  EXPECT_EQ(thickness, vertical.getSuggestedMinimumDimensions().width());
  EXPECT_EQ(0, vertical.getSuggestedMinimumDimensions().height());
  EXPECT_TRUE(horizontal.getPreferredSize().width().isMatchParent());
  EXPECT_EQ(thickness, horizontal.getPreferredSize().height().value());
  EXPECT_TRUE(vertical.getPreferredSize().height().isMatchParent());
  EXPECT_EQ(thickness, vertical.getPreferredSize().width().value());
  Dimensions h =
      horizontal.measure(WidthSpec::Exactly(40), HeightSpec::AtMost(20));
  Dimensions v =
      vertical.measure(WidthSpec::AtMost(20), HeightSpec::Exactly(40));
  EXPECT_EQ(40, h.width());
  EXPECT_EQ(thickness, h.height());
  EXPECT_EQ(thickness, v.width());
  EXPECT_EQ(40, v.height());
  EXPECT_EQ(Padding(0), horizontal.getPadding());
  EXPECT_EQ(Padding(0), vertical.getPadding());
  EXPECT_EQ(0, horizontal.getMargins().left());
  EXPECT_EQ(0, vertical.getMargins().left());
  theme_.material3.density = Density::kMinus5;
  EXPECT_EQ(thickness, horizontal.dividerThickness());
  EXPECT_EQ(thickness, vertical.dividerThickness());
}

class CustomDivider : public HorizontalDivider {
 public:
  using HorizontalDivider::HorizontalDivider;
  Color dividerColor() const override { return Color(0x8088CC44); }
  int16_t dividerThickness() const override { return thickness; }
  int16_t thickness = 3;
};

// Verifies inset placement, an alpha color override, and changed thickness
// compose in one write; dirty and full repaints produce identical pixels.
TEST_F(Material3DividerTest, OverridesLayoutAndDirtyRepaint) {
  auto column = std::make_unique<FlexLayout>(context(), FlexDirection::kColumn);
  auto divider = std::make_unique<CustomDivider>(context());
  auto* stripe = divider.get();
  column->setPadding(Padding(7, 5));
  column->add(std::move(divider));
  app_.add(std::move(column), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();
  EXPECT_EQ(7, stripe->parent_bounds().xMin());
  EXPECT_EQ(5, stripe->parent_bounds().yMin());
  EXPECT_EQ(kWidth - 14, stripe->width());
  EXPECT_EQ(3, stripe->height());
  EXPECT_EQ(
      roo_display::AlphaBlend(theme_.framework.surface, stripe->dividerColor()),
      pixel(7, 5));
  EXPECT_EQ(theme_.framework.surface, pixel(6, 5));
  stripe->thickness = 5;
  stripe->requestLayout();
  stripe->invalidateInterior();
  refresh();
  EXPECT_EQ(5, stripe->height());
  const auto expected = pixels_;
  stripe->setDirty();
  refresh();
  EXPECT_EQ(expected, pixels_);
  app_.invalidateTheme();
  refresh();
  EXPECT_EQ(expected, pixels_);
}

class CustomVerticalDivider : public VerticalDivider {
 public:
  using VerticalDivider::VerticalDivider;
  Color dividerColor() const override { return Color(0xFF446688); }
  int16_t dividerThickness() const override { return 4; }
};

// Verifies a vertical override stretches between layout insets, reserves its
// exact width, and retains its explicit ink after live theme replacement.
TEST_F(Material3DividerTest, VerticalOverridePlacement) {
  auto row = std::make_unique<FlexLayout>(context(), FlexDirection::kRow);
  auto divider = std::make_unique<CustomVerticalDivider>(context());
  auto* stripe = divider.get();
  row->setPadding(Padding(7, 5));
  row->add(std::move(divider));
  app_.add(std::move(row), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();
  EXPECT_EQ(7, stripe->parent_bounds().xMin());
  EXPECT_EQ(5, stripe->parent_bounds().yMin());
  EXPECT_EQ(4, stripe->width());
  EXPECT_EQ(kHeight - 10, stripe->height());
  theme_.material3.color.outlineVariant = Color(0xFFFF0000);
  app_.invalidateTheme();
  refresh();
  EXPECT_EQ(stripe->dividerColor(), pixel(7, 5));
  const auto expected = pixels_;
  stripe->setDirty();
  refresh();
  EXPECT_EQ(expected, pixels_);
}

// Verifies uniform stripes in both orientations track light, dark, and live
// palette colors without consulting M2, and repaint only on invalidation.
TEST_F(Material3DividerTest, LivePaletteAndBothOrientations) {
  auto horizontal = std::make_unique<HorizontalDivider>(context());
  auto vertical = std::make_unique<VerticalDivider>(context());
  app_.add(std::move(horizontal), Box(5, 5, 60, 5));
  app_.add(std::move(vertical), Box(70, 5, 70, 60));
  for (Color ink : {Color(0xFFCAC4D0), Color(0xFF49454F), Color(0xFF339977)}) {
    theme_.framework.surface =
        ink == Color(0xFF49454F) ? Color(0xFF1C1B1F) : Color(0xFFFFFBFE);
    theme_.framework.canvas = theme_.framework.surface;
    theme_.material3.color.outlineVariant = ink;
    theme_.material2.color.outline = Color(0xFFFF0000);
    app_.invalidateTheme();
    refresh();
    for (int x = 5; x <= 60; ++x) EXPECT_EQ(ink, pixel(x, 5));
    for (int y = 5; y <= 60; ++y) EXPECT_EQ(ink, pixel(70, y));
  }
}

class TallBody : public Widget {
 public:
  using Widget::Widget;
  Margins getMargins() const override { return Margins(0, 3); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(40, 300);
  }
};
class ScrollDialog : public internal::DialogScaffold {
 public:
  using Container::focusChildAt;
  ScrollDialog(ApplicationContext& context, WidgetRef body,
               internal::DialogScaffoldVariant variant)
      : DialogScaffold(context, std::move(body), variant) {}
  ~ScrollDialog() override { prepareForDerivedDestruction(); }
};

// Verifies basic/full-screen dividers retain scroll-boundary visibility and
// content insets, and use the resolved thickness for their actual rectangles.
TEST_F(Material3DividerTest, DialogScrollBoundaries) {
  for (auto variant : {internal::DialogScaffoldVariant::kBasic,
                       internal::DialogScaffoldVariant::kFullScreen}) {
    auto owned = std::make_unique<ScrollDialog>(
        context(), WidgetRef(std::make_unique<TallBody>(context())), variant);
    ScrollDialog& dialog = *owned;
    app_.add(std::move(owned), Box(0, 0, 95, 71));
    app_.refresh();
    auto& scroller =
        static_cast<SimpleScrollablePanel&>(*dialog.focusChildAt(1));
    Widget& top = *dialog.focusChildAt(2);
    Widget& bottom = *dialog.focusChildAt(3);
    EXPECT_TRUE(top.isGone());
    EXPECT_FALSE(bottom.isGone());
    EXPECT_EQ(std::max(1, Scaled(1)), bottom.height());
    EXPECT_EQ(scroller.parent_bounds().xMin(), bottom.parent_bounds().xMin());
    EXPECT_EQ(scroller.width(), bottom.width());
    scroller.scrollTo(0, -10);
    EXPECT_FALSE(top.isGone());
    EXPECT_FALSE(bottom.isGone());
    EXPECT_EQ(std::max(1, Scaled(1)), top.height());
    scroller.scrollToBottom();
    EXPECT_FALSE(top.isGone());
    EXPECT_TRUE(bottom.isGone());
    scroller.scrollToTop();
    EXPECT_TRUE(top.isGone());
    EXPECT_FALSE(bottom.isGone());
  }
}
}  // namespace
}  // namespace roo_windows::material3
