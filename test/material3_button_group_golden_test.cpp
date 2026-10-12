#include <vector>

#include "button_group_test_support.h"
#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"
namespace roo_windows::material3 {
namespace {
// Reports a useful coordinate instead of dumping complete framebuffer vectors.
void ExpectSameRaster(const std::vector<roo::byte>& incremental,
                      const std::vector<roo::byte>& full) {
  if (incremental == full) return;
  size_t first = 0;
  while (first < full.size() && incremental[first] == full[first]) ++first;
  EXPECT_TRUE(incremental == full)
      << "first differing pixel=" << first / 2 % 320 << "," << first / 2 / 320;
}
class WriteCountingDevice final
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  WriteCountingDevice(int width, int height, roo::byte* raster)
      : OffscreenDevice(width, height, raster, roo_display::Argb4444()),
        width_(width),
        writes_(width * height, 0) {}

  void reset() { std::fill(writes_.begin(), writes_.end(), 0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(roo_display::Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(roo_display::Color color, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(color, count);
  }

  void writePixels(roo_display::BlendingMode mode, roo_display::Color* values,
                   int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(roo_display::BlendingMode mode, roo_display::Color color,
                  int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(roo_display::BlendingMode mode, roo_display::Color* values,
                  int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(roo_display::BlendingMode mode, roo_display::Color color,
                 int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                 uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  uint16_t writesAt(int x, int y) const { return writes_[y * width_ + x]; }

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      if (x_ < width_ && y_ < static_cast<int>(writes_.size() / width_)) {
        ++writes_[y_ * width_ + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int width_;
  std::vector<uint16_t> writes_;
  int x_ = 0;
  int y_ = 0;
  int left_ = 0;
  int right_ = 0;
};

class GoldenPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class PaletteSurface : public SurfaceWidget {
 public:
  PaletteSurface(ApplicationContext& context, internal::GroupStyle style,
                 bool selected, bool enabled)
      : SurfaceWidget(context),
        style_(style),
        selected_(selected),
        enabled_(enabled) {}
  Color background() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .container;
  }
  Color getOutlineColor() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .outline;
  }
  BorderStyle getBorderStyle() const override {
    return internal::ResolveConnectedBorder(
        ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
        true, false, selected_, 0, {width(), height()},
        style_ == internal::GroupStyle::kOutlined && !selected_ ? Scaled(1)
                                                                : 0);
  }
  bool useAutomaticDisabledStyle() const override { return false; }
  Dimensions getSuggestedMinimumDimensions() const override { return {60, 40}; }

 private:
  internal::GroupStyle style_;
  bool selected_;
  bool enabled_;
};
// Verifies real surface rendering for each palette and selection combination.
TEST(ButtonGroupGolden, ConnectedHelperSurfaces) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    GoldenPanel panel(app.context());
    std::vector<std::unique_ptr<PaletteSurface>> entries;
    app.addTaskFullScreen(panel);
    for (int i = 0; i < 12; ++i) {
      entries.push_back(std::make_unique<PaletteSurface>(
          app.context(), static_cast<internal::GroupStyle>(i % 4), i >= 4,
          i < 8));
      panel.add(WidgetRef(*entries.back()),
                Rect(8 + (i % 4) * 76, 8 + (i / 4) * 48, 67 + (i % 4) * 76,
                     47 + (i / 4) * 48));
    }
    app.refresh();
    std::string name = dark ? "helper_dark" : "helper_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 160),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    panel.removeAll();
  }
}

class StripedPanel : public GoldenPanel {
 public:
  using GoldenPanel::GoldenPanel;
  void paint(PaintContext& context) const override {
    for (int x = 0; x < width(); x += 8) {
      context.canvas().fillRect(x, 0, std::min<int>(width() - 1, x + 7),
                                height() - 1,
                                x % 16 == 0 ? roo_display::Color(0xFF335577)
                                            : roo_display::Color(0xFF667788));
    }
  }
};
// Verifies stable mixed entries preserve patterned gaps and default clipping.
TEST(ButtonGroupGolden, InternalLayoutAndRestoration) {
  for (bool unclipped : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    StripedPanel panel(app.context());
    MonoIcon icon = ic_outlined_24_action_home();
    internal::ButtonGroupLabelEntry day(app.context(), "Day",
                                        internal::GroupStyle::kElevated);
    internal::ButtonGroupIconEntry home(app.context(), icon,
                                        internal::GroupStyle::kOutlined);
    internal::ButtonGroupLabelEntry week(app.context(), "Week");
    test_support::InternalGroup group(app.context());
    group.add(day);
    group.add(home);
    group.add(week);
    if (unclipped) day.setParentClipMode(ParentClipMode::kUnclipped);
    app.addTaskFullScreen(panel);
    panel.add(WidgetRef(group), Rect(24, 24, 183, 73));
    app.refresh();
    std::string name = unclipped ? "internal_unclipped" : "internal_clipped";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 160),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    for (LayoutDirection direction :
         {LayoutDirection::kRightToLeft, LayoutDirection::kLeftToRight}) {
      home.setVisibility(direction == LayoutDirection::kRightToLeft
                             ? Visibility::kGone
                             : Visibility::kVisible);
      group.setLayoutDirection(direction);
      app.refresh();
      std::vector<roo::byte> incremental = raster;
      app.root().invalidateDescending();
      app.refresh();
      ExpectSameRaster(incremental, raster);
    }
    panel.removeAll();
  }
}

// Verifies selected/pressed/disabled contours and stable anchors across each
// contained style, including mixed artwork, RTL, compact density and dark mode.
TEST(ButtonGroupGolden, ConnectedSelectionAndRestoration) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 240 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 240, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    StripedPanel panel(app.context());
    app.addTaskFullScreen(panel);
    MonoIcon icon = ic_outlined_24_action_home();
    std::vector<std::unique_ptr<ConnectedButtonGroup>> groups;
    for (int i = 0; i < 4; ++i) {
      auto group = std::make_unique<ConnectedButtonGroup>(app.context());
      group->add(std::make_unique<ButtonGroupButton>(
          app.context(), "Day", static_cast<ButtonGroupButtonStyle>(i)));
      auto middle = std::make_unique<ButtonGroupButton>(
          app.context(), "Week", static_cast<ButtonGroupButtonStyle>(i));
      middle->setIcon(&icon);
      group->add(std::move(middle));
      group->add(std::make_unique<ButtonGroupButton>(
          app.context(), "Month", static_cast<ButtonGroupButtonStyle>(i)));
      group->setSelectionMask(2);
      panel.add(WidgetRef(*group), Rect(12, 12 + i * 52, 307, 55 + i * 52));
      groups.push_back(std::move(group));
    }
    app.refresh();
    std::string name = dark ? "connected_dark" : "connected_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 240),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    for (uint32_t mask : {1u, 4u, 2u}) {
      for (auto& group : groups) {
        Widget* child = static_cast<Widget&>(*group).focusChildAt(1);
        Rect previous = child->parent_bounds();
        group->setSelectionMask(mask);
        EXPECT_EQ(previous, child->parent_bounds());
        child->setPressed(mask == 4);
      }
      app.refresh();
      std::vector<roo::byte> incremental = raster;
      app.root().invalidateDescending();
      app.refresh();
      ExpectSameRaster(incremental, raster);
    }
    material.density = Density::kMinus3;
    for (auto& group : groups) {
      group->setLayoutDirection(LayoutDirection::kRightToLeft);
      group->setShape(ButtonShape::kSquare);
      group->setEnabled(false);
    }
    panel.requestLayoutDescending();
    app.root().invalidateDescending();
    app.refresh();
    name =
        dark ? "connected_disabled_rtl_dark" : "connected_disabled_rtl_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 240),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    panel.removeAll();
  }
}

// Verifies gone endpoints update surviving corners even when constrained
// natural frames remain unchanged; invisible slots retain their corner role.
TEST(ButtonGroupGolden, ConstrainedGoneEndpointRestoration) {
  std::vector<roo::byte> raster(320 * 160 * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(320, 160,
                                                             raster.data(), {});
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  StripedPanel panel(app.context());
  ButtonGroupButton day(app.context(), "Day");
  ButtonGroupButton week(app.context(), "Week");
  ButtonGroupButton month(app.context(), "Month");
  ConnectedButtonGroup group(app.context());
  group.add(day);
  group.add(week);
  group.add(month);
  app.addTaskFullScreen(panel);
  const int strip_width = day.entryDimensions().width() +
                          week.entryDimensions().width() + Scaled(2);
  panel.add(WidgetRef(group), Rect(24, 40, 23 + strip_width, 79));
  app.refresh();
  Rect before = week.parent_bounds();
  for (Visibility visibility :
       {Visibility::kInvisible, Visibility::kGone, Visibility::kVisible}) {
    month.setVisibility(visibility);
    app.refresh();
    EXPECT_EQ(before, week.parent_bounds());
    std::vector<roo::byte> incremental = raster;
    app.root().invalidateDescending();
    app.refresh();
    ExpectSameRaster(incremental, raster);
  }
  group.setShape(ButtonShape::kSquare);
  app.refresh();
  std::vector<roo::byte> incremental = raster;
  app.root().invalidateDescending();
  app.refresh();
  ExpectSameRaster(incremental, raster);
  panel.removeAll();
}
// Verifies transparent groups never prefill the strip: every pixel is emitted
// once with its final color, including rounded corners and gap restoration.
TEST(ButtonGroupGolden, FinalPixelWritesDuringMotion) {
  std::vector<roo::byte> raster(320 * 160 * 2);
  WriteCountingDevice device(320, 160, raster.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  StripedPanel panel(app.context());
  ButtonGroupButton left(app.context(), "Capture");
  ButtonGroupButton right(app.context(), "Save");
  ButtonGroup group(app.context());
  group.add(left);
  group.add(right);
  app.addTaskFullScreen(panel);
  panel.add(WidgetRef(group), Rect(24, 40, 295, 95));
  app.refresh();
  left.onShowPress(10, 20);
  for (int step = 0; step < 3; ++step) {
    device.reset();
    system_time_delay_micros(kPressAnimationMillis * 100);
    app.refresh();
    for (int y = 0; y < 160; ++y)
      for (int x = 0; x < 320; ++x)
        ASSERT_LE(device.writesAt(x, y), 1) << "at " << x << "," << y;
  }
  left.onCancel();
  device.reset();
  app.refresh();
  for (int y = 0; y < 160; ++y)
    for (int x = 0; x < 320; ++x)
      ASSERT_LE(device.writesAt(x, y), 1) << "at " << x << "," << y;
  panel.removeAll();
}
// Verifies moving standard frames and unclipped elevated shadows restore
// correctly at intermediate samples, cancellations and content changes.
TEST(ButtonGroupGolden, StandardMotionRestoration) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    StripedPanel panel(app.context());
    ButtonGroupButton capture(app.context(), "Capture",
                              ButtonGroupButtonStyle::kElevated);
    ButtonGroupButton save(app.context(), "Save",
                           ButtonGroupButtonStyle::kFilledTonal);
    MonoIcon icon = ic_outlined_24_action_home();
    ButtonGroupIconButton home(app.context(), icon,
                               ButtonGroupIconStyle::kOutlined);
    ButtonGroup group(app.context());
    group.add(capture);
    group.add(save);
    group.add(home);
    capture.setParentClipMode(ParentClipMode::kUnclipped);
    app.addTaskFullScreen(panel);
    panel.add(WidgetRef(group), Rect(24, 40, 295, 95));
    app.refresh();
    for (Widget* target :
         {static_cast<Widget*>(&capture), static_cast<Widget*>(&save),
          static_cast<Widget*>(&home)}) {
      target->onShowPress(10, 20);
      for (int step = 0; step < 4; ++step) {
        system_time_delay_micros(kPressAnimationMillis * 80);
        app.refresh();
        std::vector<roo::byte> incremental = raster;
        app.root().invalidateDescending();
        app.refresh();
        SCOPED_TRACE(::testing::Message() << "step=" << step << " target="
                                          << (target == &capture ? "capture"
                                              : target == &save  ? "save"
                                                                 : "home"));
        ExpectSameRaster(incremental, raster);
      }
      if (target == &save) {
        std::string name =
            dark ? "standard_pressed_dark" : "standard_pressed_light";
        EXPECT_TRUE(test::CompareOrUpdateGolden(
            test::CaptureRgb(device.raster(), 0, 0, 320, 160),
            "test/goldens/material3_button_group/" + name + ".ppm", name));
        save.setLabel("Store");
        app.refresh();
        std::vector<roo::byte> incremental = raster;
        app.root().invalidateDescending();
        app.refresh();
        ExpectSameRaster(incremental, raster);
      }
      target->onCancel();
      app.refresh();
      std::vector<roo::byte> incremental = raster;
      app.root().invalidateDescending();
      app.refresh();
      ExpectSameRaster(incremental, raster);
    }
    group.setLayoutDirection(LayoutDirection::kRightToLeft);
    group.setShapeMorph(ButtonShapeMorph::kDisabled);
    app.refresh();
    std::vector<roo::byte> incremental = raster;
    app.root().invalidateDescending();
    app.refresh();
    ExpectSameRaster(incremental, raster);
    panel.removeAll();
  }
}
}  // namespace
}  // namespace roo_windows::material3
