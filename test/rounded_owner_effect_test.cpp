#include <array>
#include <type_traits>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows.h"
#include "roo_windows/core/paint_effect.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {
namespace {

using roo_display::AlphaBlend;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
constexpr int kWidth = 96;
constexpr int kHeight = 72;
constexpr Color kBackdrop(0xFF3B5A43);
constexpr Color kPanel(0xFFE2D7C5);
constexpr Color kChild(0xFF396AC5);
constexpr Color kOutline(0xFF632936);
constexpr Color kOverlay(0x9678C62A);

// Counts actual device writes across every output entry point, after filters.
template <typename Format>
class EffectDevice : public roo_display::OffscreenDevice<Format> {
 public:
  using OffscreenDevice = roo_display::OffscreenDevice<Format>;
  explicit EffectDevice(roo::byte* data)
      : OffscreenDevice(kWidth, kHeight, data, Format()) {}

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

class EffectPanel : public Panel {
 public:
  using Panel::add;
  EffectPanel(ApplicationContext& context, Color color, BorderStyle border,
              bool rounded = true)
      : Panel(context), color_(color), border_(border), rounded_(rounded) {}
  Color background() const override { return color_; }
  BorderStyle getBorderStyle() const override { return border_; }
  Color getOutlineColor() const override { return kOutline; }
  uint8_t getElevation() const override { return rounded_ ? 2 : 0; }
  bool clipsChildrenToRoundedBounds() const override { return rounded_; }
  bool isClickable() const override { return true; }
  void setRounded(bool rounded) {
    rounded_ = rounded;
    invalidateInterior();
  }

 private:
  Color color_;
  BorderStyle border_;
  bool rounded_;
};

class EffectLeaf : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
  Color background() const override { return kChild; }
  BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }
  bool isClickable() const override { return true; }
  void paint(PaintContext& ctx) const override {
    ++paints;
    ctx.clear();
  }
  mutable int paints = 0;
};

class EffectForeground : public Widget {
 public:
  using Widget::Widget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
  void paint(PaintContext& ctx) const override {
    ++paints;
    ctx.addOverlayShape(roo_display::SmoothFilledRoundRect(
        0, 0, width() - 1, height() - 1, 6, kOverlay));
  }
  mutable int paints = 0;

 protected:
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
};

// Test-only reference: compose complete child layers over the owner's fill,
// transform the resulting group, then rasterize its outline and coverage.
// It uses Decoration as the independent geometry oracle, never the clipper's
// routing, sparse colors, masks, or PaintEffect implementation.
struct ReferenceNode {
  SurfaceWidget* surface;
  Widget* widget;
  std::vector<ReferenceNode> children;
  bool rounded_owner = true;
  Box bounds{0, 0, -1, -1};
  Color background;
  OverlaySpec spec;
  PressOverlay ripple;

  explicit ReferenceNode(SurfaceWidget& surface)
      : surface(&surface), widget(&surface) {}
  explicit ReferenceNode(EffectForeground& foreground)
      : surface(nullptr), widget(&foreground) {}

  void sample(roo_display::DisplayOutput& output, Color parent_background) {
    XDim dx;
    YDim dy;
    widget->getAbsoluteOffset(dx, dy);
    bounds = Box(dx, dy, dx + widget->width() - 1, dy + widget->height() - 1);
    background = parent_background;
    if (surface != nullptr) {
      Color fill = surface->background();
      if (!surface->isEnabled()) fill.set_a(fill.a() / 2);
      background = AlphaBlend(parent_background, fill);
    }
    roo_display::Surface target(output, dx, dy, bounds, false, background,
                                roo_display::FillMode::kVisible,
                                BlendingMode::kSourceOver);
    spec = OverlaySpec(*widget, Canvas(&target));
    if (spec.has_press_overlay()) {
      const PressOverlaySpec& p = spec.press_overlay();
      ripple = PressOverlay(p.center_x, p.center_y, p.radius, p.color);
    }
    for (ReferenceNode& child : children) child.sample(output, background);
  }

  Color transform(Color color, int16_t x, int16_t y) const {
    if (spec.is_disabled()) {
      color.set_a(
          (color.a() *
           widget->theme().framework.interaction.disabledContentOpacity) >>
          7);
      return AlphaBlend(background, color);
    }
    if (!spec.is_area()) return color;
    return AlphaBlend(color, spec.has_press_overlay() ? ripple.get(x, y)
                                                      : spec.base_overlay());
  }

  Color at(int16_t x, int16_t y) const {
    if (surface == nullptr) {
      auto shape = roo_display::SmoothFilledRoundRect(
          bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax(), 6,
          kOverlay);
      Color c;
      shape.readColors(&x, &y, 1, &c);
      return c;
    }
    const BorderStyle border =
        surface->getBorderStyle().trim(bounds.width(), bounds.height());
    Color outline = AlphaBlend(background, surface->getOutlineColor());
    // Ordinary child surfaces retain their established Decoration semantics.
    if (!rounded_owner && spec.has_press_overlay()) {
      Decoration layer(bounds, surface->getElevation(), spec, &ripple,
                       background, border.corner_radii(),
                       border.outline_width(), outline);
      Color result;
      layer.readColors(&x, &y, 1, &result);
      return result;
    }
    Color content = background;
    const int inset = border.outline_width().ceil();
    const Box viewport(bounds.xMin() + inset, bounds.yMin() + inset,
                       bounds.xMax() - inset, bounds.yMax() - inset);
    for (const ReferenceNode& child : children) {
      if (viewport.contains(x, y) &&
          child.widget->getParentClipMode() == ParentClipMode::kClipped) {
        content = AlphaBlend(content, child.at(x, y));
      }
    }
    content = transform(content, x, y);
    outline = transform(outline, x, y);
    Decoration layer(bounds, surface->getElevation(), OverlaySpec(), nullptr,
                     content, border.corner_radii(), border.outline_width(),
                     outline, true);
    Color result;
    layer.readColors(&x, &y, 1, &result);
    for (const ReferenceNode& child : children) {
      if (child.widget->getParentClipMode() == ParentClipMode::kUnclipped) {
        Color foreground = child.at(x, y);
        foreground =
            transform(foreground.withA(255), x, y).withA(foreground.a());
        result = AlphaBlend(result, foreground);
      }
    }
    return result;
  }
};

template <typename Format>
class RoundedOwnerEffectTest : public testing::Test {
 protected:
  RoundedOwnerEffectTest()
      : device_(pixels_.data()),
        display_(device_),
        env_(scheduler_),
        app_(&env_, display_) {}
  void SetUp() override {
    app_.refresh();
    auto host = std::make_unique<EffectPanel>(app_.context(), kBackdrop,
                                              BorderStyle(0, 0), false);
    host_ = host.get();
    app_.add(std::move(host), Box(0, 0, kWidth - 1, kHeight - 1));
  }

  Color pixel(int16_t x, int16_t y) const {
    Color result;
    device_.raster().readColors(&x, &y, 1, &result);
    return result;
  }

  void check(ReferenceNode& reference) {
    reference.sample(device_, kBackdrop);
    const bool rgb565 = std::is_same<Format, roo_display::Rgb565>::value;
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        Format format;
        const Color expected = format.toArgbColor(
            format.fromArgbColor(AlphaBlend(kBackdrop, reference.at(x, y))));
        const Color actual = pixel(x, y);
        // Compare RGB565 channel codes: one step expands to either eight or
        // nine RGB units, so an eight-unit tolerance is not sufficient.
        EXPECT_NEAR(actual.r() >> (rgb565 ? 3 : 0),
                    expected.r() >> (rgb565 ? 3 : 0), rgb565 ? 1 : 2)
            << x << ',' << y;
        EXPECT_NEAR(actual.g() >> (rgb565 ? 2 : 0),
                    expected.g() >> (rgb565 ? 2 : 0), rgb565 ? 1 : 2)
            << x << ',' << y;
        EXPECT_NEAR(actual.b() >> (rgb565 ? 3 : 0),
                    expected.b() >> (rgb565 ? 3 : 0), rgb565 ? 1 : 2)
            << x << ',' << y;
        EXPECT_EQ(actual.a(), expected.a());
        EXPECT_LE(device_.writes[y * kWidth + x], 1) << x << ',' << y;
      }
    }
  }

  std::array<roo::byte, kWidth * kHeight * 4> pixels_{};
  EffectDevice<Format> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
  EffectPanel* host_;
};

using Formats = testing::Types<roo_display::Argb8888, roo_display::Rgb565>;
TYPED_TEST_SUITE(RoundedOwnerEffectTest, Formats);

// Verifies owner effects reach direct children, deferred foreground, outline,
// and fractional boundaries once, including clipped/unclipped group changes.
TYPED_TEST(RoundedOwnerEffectTest, OwnerEffectsMatchComposedGroup) {
  auto panel = std::make_unique<EffectPanel>(
      this->app_.context(), kPanel,
      BorderStyle(16, 8, 12, 10, SmallNumber(1.5)));
  EffectPanel* owner = panel.get();
  auto child = std::make_unique<EffectLeaf>(this->app_.context());
  EffectLeaf* leaf = child.get();
  panel->add(std::move(child), Rect(-4, -4, 61, 26));
  auto foreground = std::make_unique<EffectForeground>(this->app_.context());
  EffectForeground* overlay = foreground.get();
  panel->add(std::move(foreground), Rect(-8, 8, 45, 36));
  this->host_->add(std::move(panel), Rect(16, 14, 79, 57));
  ReferenceNode reference(*owner);
  reference.children.emplace_back(*leaf);
  reference.children.back().rounded_owner = false;
  reference.children.emplace_back(*overlay);
  for (int frame = 0; frame < 7; ++frame) {
    SCOPED_TRACE(testing::Message() << "frame=" << frame);
    if (frame == 1) {
      owner->setPressed(true);
      leaf->setPressed(true);
    }
    if (frame == 2) {
      overlay->setParentClipMode(ParentClipMode::kUnclipped);
    }
    if (frame == 3) {
      owner->setPressed(false);
      owner->onShowPress(3, 10);
      delay(80);
    }
    if (frame == 4) {
      owner->setEnabled(false);
    }
    if (frame == 5) {
      owner->setEnabled(true);
      owner->setPressed(false);
      leaf->setEnabled(false);
    }
    if (frame == 6) {
      owner->setPressed(true);
      leaf->setEnabled(true);
      leaf->setPressed(false);
      leaf->onShowPress(6, 8);
      delay(80);
    }
    this->device_.reset();
    this->app_.refresh();
    this->check(reference);
    if (frame == 3) {
      EXPECT_TRUE(reference.spec.has_press_overlay());
    }
    if (frame == 6) {
      EXPECT_TRUE(reference.children[0].spec.has_press_overlay());
    }
    EXPECT_EQ(leaf->paints, frame + 1);
    EXPECT_EQ(overlay->paints, frame + 1);
  }
}

// Verifies an inner rounded owner and its escaped foreground inherit outer
// effects, while the outer mask still clips the entire inner subtree.
TYPED_TEST(RoundedOwnerEffectTest, NestedOwnersAndEscapedForeground) {
  auto outer = std::make_unique<EffectPanel>(this->app_.context(), kPanel,
                                             BorderStyle(16, 0));
  EffectPanel* owner = outer.get();
  auto inner = std::make_unique<EffectPanel>(this->app_.context(), kChild,
                                             BorderStyle(10, SmallNumber(1.5)));
  EffectPanel* inner_owner = inner.get();
  auto foreground = std::make_unique<EffectForeground>(this->app_.context());
  EffectForeground* overlay = foreground.get();
  overlay->setParentClipMode(ParentClipMode::kUnclipped);
  inner->add(std::move(foreground), Rect(-6, -8, 35, 13));
  outer->add(std::move(inner), Rect(-5, -3, 42, 32));
  this->host_->add(std::move(outer), Rect(16, 14, 79, 57));
  ReferenceNode reference(*owner);
  reference.children.emplace_back(*inner_owner);
  reference.children.back().children.emplace_back(*overlay);
  for (int frame = 0; frame < 4; ++frame) {
    SCOPED_TRACE(testing::Message() << "frame=" << frame);
    if (frame == 0) {
      owner->setPressed(true);
      inner_owner->setPressed(true);
    }
    if (frame == 1) {
      inner_owner->setPressed(false);
      inner_owner->onShowPress(8, 5);
      delay(80);
    }
    if (frame == 2) {
      owner->setEnabled(false);
    }
    if (frame == 3) {
      owner->setEnabled(true);
      inner_owner->setEnabled(false);
    }
    this->device_.reset();
    this->app_.refresh();
    this->check(reference);
    if (frame == 1) {
      EXPECT_TRUE(reference.children[0].spec.has_press_overlay());
    }
  }
}

// Verifies rounded clipping leaves the ordinary owner's interior effect order
// unchanged for both output formats and owner tint/disabled styles.
TYPED_TEST(RoundedOwnerEffectTest, InteriorMatchesOrdinaryOwner) {
  auto panel = std::make_unique<EffectPanel>(this->app_.context(), kPanel,
                                             BorderStyle(16, 0));
  EffectPanel* owner = panel.get();
  auto child = std::make_unique<EffectLeaf>(this->app_.context());
  child->setPressed(true);
  panel->add(std::move(child), Rect(4, 4, 59, 39));
  this->host_->add(std::move(panel), Rect(16, 14, 79, 57));
  for (bool disabled : {false, true}) {
    owner->setRounded(true);
    owner->setPressed(true);
    owner->setEnabled(!disabled);
    this->app_.refresh();
    std::vector<Color> before;
    for (int y = 30; y < 42; ++y) {
      for (int x = 32; x < 64; ++x) before.push_back(this->pixel(x, y));
    }
    owner->setRounded(false);
    this->device_.reset();
    this->app_.refresh();
    size_t i = 0;
    for (int y = 30; y < 42; ++y) {
      for (int x = 32; x < 64; ++x) {
        EXPECT_EQ(this->pixel(x, y), before[i++]);
      }
    }
  }
}

}  // namespace
}  // namespace roo_windows
