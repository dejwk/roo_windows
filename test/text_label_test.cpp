#include "roo_windows/widgets/text_label.h"

#include <algorithm>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/ui/text_label.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

constexpr char kOverhangText[] = "jQy/";

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

FlexLayout::Params FillWidthParams() {
  FlexLayout::Params params;
  params.flex_grow = 1;
  params.flex_basis = FlexBasis::kZero;
  return params;
}

struct PixelBounds {
  bool found;
  int16_t x_min;
  int16_t y_min;
  int16_t x_max;
  int16_t y_max;
};

class TextLabelRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 96;
  static constexpr int16_t kHeight = 64;

  TextLabelRenderTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  PixelBounds findNonBackground(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                Color background) const {
    PixelBounds out = {false, 0, 0, 0, 0};
    for (int16_t y = y0; y <= y1; ++y) {
      for (int16_t x = x0; x <= x1; ++x) {
        if (pixelAt(x, y) == background) continue;
        if (!out.found) {
          out.found = true;
          out.x_min = out.x_max = x;
          out.y_min = out.y_max = y;
        } else {
          out.x_min = std::min(out.x_min, x);
          out.y_min = std::min(out.y_min, y);
          out.x_max = std::max(out.x_max, x);
          out.y_max = std::max(out.y_max, y);
        }
      }
    }
    return out;
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

class TextLabelGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 128;
  static constexpr int16_t kHeight = 80;
  static constexpr int16_t kLayoutX = 8;
  static constexpr int16_t kLayoutY = 8;
  static constexpr int16_t kLayoutWidth = 96;
  static constexpr int16_t kLayoutHeight = 40;

  TextLabelGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  Offscreen<Rgb888> RenderFlexLabel(Gravity gravity,
                                    const FlexLayout::Params& params,
                                    PaddingSize padding = PaddingSize::kNone) {
    FlexLayout layout(env_, FlexDirection::kRow);
    layout.setPadding(Padding(12, 8));
    layout.setJustifyContent(JustifyContent::kCenter);
    layout.setAlignItems(AlignItems::kCenter);

    TextLabel label(env_, kOverhangText, font_body2(), gravity);
    label.setPadding(padding);
    layout.add(label, params);

    Application app(&env_, display_);
    app.add(layout, Box(kLayoutX, kLayoutY, kLayoutX + kLayoutWidth - 1,
                        kLayoutY + kLayoutHeight - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), kLayoutX, kLayoutY,
                            kLayoutWidth, kLayoutHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(const Environment& env) : Panel(env) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

}  // namespace

TEST(TextLabel, SuggestedMinimumDimensionsMatchFontMetrics) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();
  TextLabel label(env, "abc", font);

  auto metrics = font.getHorizontalStringMetrics("abc");
  Dimensions dims = label.getSuggestedMinimumDimensions();
  EXPECT_EQ(metrics.advance(), dims.width());
  EXPECT_EQ(font.metrics().ascent() - font.metrics().descent() +
                font.metrics().linegap(),
            dims.height());
}

TEST(TextLabel, ContentBoundsFollowDrawableInkExtents) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();
  TextLabel label(env, "abc", font, kGravityLeft | kGravityMiddle);
  label.setPadding(PaddingSize::kNone);

  Dimensions dims = label.getSuggestedMinimumDimensions();
  label.layout(Rect(0, 0, dims.width() - 1, dims.height() - 1));

  auto metrics = font.getHorizontalStringMetrics("abc");
  Rect anchor_bounds(0, -font.metrics().ascent() - font.metrics().linegap(),
                     metrics.advance() - 1, -font.metrics().descent());
  auto offset = ResolveAlignmentOffset(label.bounds(), anchor_bounds,
                                       roo_display::kLeft | roo_display::kMiddle);
  Rect expected = Rect(metrics.screen_extents()).translate(offset.first,
                                                           offset.second);

  EXPECT_EQ(expected, label.getContentBounds());
}

TEST(TextLabel, EmptyTextHasZeroInkInsets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();

  TextLabel label(env, "", font, kGravityLeft | kGravityMiddle);
  StringViewLabel string_view_label(env, roo::string_view(), font,
                                    kGravityLeft | kGravityMiddle);

  EXPECT_EQ(Insets::Zero(), label.getInkInsets());
  EXPECT_EQ(Insets::Zero(), string_view_label.getInkInsets());
}

TEST(TextLabel, EmptyToNonEmptyDoesNotInvalidateParentBeneath) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  RecordingPanel panel(env);

  auto label = std::make_unique<TextLabel>(env, "", font_body2(),
                                           kGravityCenter | kGravityMiddle);
  TextLabel* label_ptr = label.get();
  panel.add(std::move(label), Rect(0, 0, 119, 19));
  panel.invalidated_regions.clear();

  label_ptr->setText("42.0");

  EXPECT_TRUE(panel.invalidated_regions.empty());
}

TEST(TextLabel, TextChangeInvalidatesOnlyOldVisualBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  RecordingPanel panel(env);

  auto label = std::make_unique<TextLabel>(env, "A", font_body2(),
                                           kGravityCenter | kGravityMiddle);
  TextLabel* label_ptr = label.get();
  panel.add(std::move(label), Rect(0, 0, 119, 19));
  Rect old_bounds = label_ptr->maxParentBounds();
  panel.invalidated_regions.clear();

  label_ptr->setText("MMMMMMMM");

  ASSERT_EQ(1u, panel.invalidated_regions.size());
  EXPECT_EQ(old_bounds, panel.invalidated_regions.front());
}

TEST_F(TextLabelRenderTest, SetTextAndClearTextAffectRenderedPixels) {
  auto label = std::make_unique<TextLabel>(env_, "A", font_body2());
  TextLabel* label_ptr = label.get();
  app_.add(std::move(label), Box(8, 8, 80, 32));

  ASSERT_TRUE(refresh());
  Color bg = QuantizeToArgb4444(env_.theme().color.background);
  PixelBounds initial = findNonBackground(8, 8, 80, 32, bg);
  ASSERT_TRUE(initial.found);

  label_ptr->setText("MMMMMMMM");
  ASSERT_TRUE(refresh());
  PixelBounds expanded = findNonBackground(8, 8, 80, 32, bg);
  ASSERT_TRUE(expanded.found);
  EXPECT_GT(expanded.x_max - expanded.x_min, initial.x_max - initial.x_min);

  label_ptr->clearText();
  ASSERT_TRUE(refresh());
  PixelBounds cleared = findNonBackground(8, 8, 80, 32, bg);
  EXPECT_FALSE(cleared.found);
}

TEST_F(TextLabelRenderTest, GravityChangesHorizontalPlacement) {
  auto left = std::make_unique<TextLabel>(env_, "WWW", font_body2(),
                                          kGravityLeft | kGravityMiddle);
  auto right = std::make_unique<TextLabel>(env_, "WWW", font_body2(),
                                           kGravityRight | kGravityMiddle);
  left->setPadding(PaddingSize::kNone);
  right->setPadding(PaddingSize::kNone);

  app_.add(std::move(left), Box(4, 6, 44, 28));
  app_.add(std::move(right), Box(4, 34, 44, 56));

  ASSERT_TRUE(refresh());
  Color bg = QuantizeToArgb4444(env_.theme().color.background);
  PixelBounds left_bounds = findNonBackground(4, 6, 44, 28, bg);
  PixelBounds right_bounds = findNonBackground(4, 34, 44, 56, bg);

  ASSERT_TRUE(left_bounds.found);
  ASSERT_TRUE(right_bounds.found);
  EXPECT_GT(right_bounds.x_min, left_bounds.x_min);
}

TEST_F(TextLabelRenderTest, TransparentColorMatchesExplicitDefaultColor) {
  auto implicit = std::make_unique<TextLabel>(env_, "Hi", font_body2(),
                                              kGravityLeft | kGravityMiddle);
  auto explicit_default = std::make_unique<TextLabel>(
      env_, "Hi", font_body2(), env_.theme().color.onBackground,
      kGravityLeft | kGravityMiddle);

  app_.add(std::move(implicit), Box(4, 6, 44, 28));
  app_.add(std::move(explicit_default), Box(4, 34, 44, 56));

  ASSERT_TRUE(refresh());
  Color bg = QuantizeToArgb4444(env_.theme().color.background);
  PixelBounds top = findNonBackground(4, 6, 44, 28, bg);
  PixelBounds bottom = findNonBackground(4, 34, 44, 56, bg);
  ASSERT_TRUE(top.found);
  ASSERT_TRUE(bottom.found);

  int16_t dx = top.x_min - 4;
  int16_t dy = top.y_min - 6;
  EXPECT_EQ(pixelAt(top.x_min, top.y_min), pixelAt(4 + dx, 34 + dy));
}

TEST_F(TextLabelGoldenTest, CenteredNaturalSizeOverhangGolden) {
  auto image =
      RenderFlexLabel(kGravityCenter | kGravityMiddle, FlexLayout::Params{});

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_center_natural.ppm",
      "text_label_flex_center_natural"));
}

TEST_F(TextLabelGoldenTest, FillWidthLeftGravityOverhangGolden) {
  auto image = RenderFlexLabel(kGravityLeft | kGravityMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_left.ppm",
      "text_label_flex_fill_left"));
}

TEST_F(TextLabelGoldenTest, FillWidthCenterGravityOverhangGolden) {
  auto image = RenderFlexLabel(kGravityCenter | kGravityMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_center.ppm",
      "text_label_flex_fill_center"));
}

TEST_F(TextLabelGoldenTest, FillWidthRightGravityOverhangGolden) {
  auto image = RenderFlexLabel(kGravityRight | kGravityMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_right.ppm",
      "text_label_flex_fill_right"));
}

TEST_F(TextLabelGoldenTest, FillWidthLeftGravityWithWidgetPaddingGolden) {
  auto image = RenderFlexLabel(kGravityLeft | kGravityMiddle,
                               FillWidthParams(), PaddingSize::kSmall);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_left_widget_padding.ppm",
      "text_label_flex_fill_left_widget_padding"));
}

}  // namespace roo_windows
